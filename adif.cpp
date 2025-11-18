#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "QLogBook.h"
#include "QMessageBox"

#include "adiffields.c"

#define min(a,b)(a<b?a:b)

#define LENFIELDSZ 10
#define TAGNAMESZ 128
#define TAGVALSZ 128

long find_next_eor(char *buf, long start, long end);
int parse_qso(char *buf, long start, long end);
void get_positions(char *tag, char **len, char **gtsign, char **nexttag);
void fill_adif_record(char *field, char *value);

// read adif file into buffer, starting import
int import_adif(char *filename) {
    FILE *fp;
    int nr = 0;
    long soc = 0, eor = 0;
    off_t len = 0;
    char *buf = NULL, *p;
    struct stat st;

    if ( (fp = fopen(filename, "r")) == NULL ) {
//        message(MSG_WARN, "open adif", errno);
        QMessageBox msgbox( QMessageBox::Warning, QString("File Open Warning"),
                           QString("Can not open adif file: %1(%2)").arg(QString(filename)).arg(errno) );
        msgbox.exec();
        return(0);
    }

    if ((fstat(fileno(fp), &st))) {
//        message(MSG_WARN, "can not stat file", errno);
        QMessageBox msgbox( QMessageBox::Warning, "File Stat Warning",
                           QString("Can not stat adif file: %1(%2)").arg(filename).arg(errno) );
        msgbox.exec();
        return(0);
    }


    buf = (char *)malloc(st.st_size + 1);
    if ( buf == NULL ) {
//        message(MSG_WARN, "malloc filesize", errno);
        QMessageBox msgbox( QMessageBox::Warning, "Malloc Error",
               QString("Can not allocate memory for adif file: %1(%2)").arg(filename).arg(errno) );
        msgbox.exec();
        return(0);
    }
    memset(buf, 0, st.st_size + 1);

    len = fread(buf, 1, st.st_size, fp);
    if ( len != st.st_size ) {
//        message(MSG_WARN, "short read", errno);
#ifdef DEBUG
        fprintf(stderr, "len=%ld, st.st_size=%ld\n", len, st.st_size);
#endif
        QMessageBox msgbox( QMessageBox::Warning, "Short Read",
               QString("Can not read file fully (short read): %1(%2)").arg(filename).arg(errno) );
        msgbox.exec();
        return(0);
    }
    fclose(fp);
    buf[st.st_size] = 0;


    create_table_if_not_exist();


    for ( off_t li = 0; li < st.st_size; li++ ) {
        if ( buf[li] > ' ' && !isprint( buf[li] ) )
            buf[li] = '*';
        else if ( buf[li] == '\'')
            buf[li] = '`';
    }

    soc = 0; // start of call mostly <call>
    p = strcasestr(buf, "<eoh>");
    if ( p )
        soc = (p - buf) + 5;
    eor = st.st_size; // <eor>
    nr = 0;

    do {
        eor = find_next_eor(buf, soc, eor);
        if ( eor > 0 && eor < st.st_size ) {
            nr += parse_qso(buf, soc, eor);
        }
        soc = eor + 1;
    } while ( soc < (st.st_size - 20)); // 20 less than size just for safety

    free(buf);
    return(nr);
}

// find next <eor> in stream
long find_next_eor(char *buf, long start, long end) {
    char *p;

    p = strcasestr((buf+start), "<eor>");
    if ( p == NULL ) {
        return(end);
    } else {
        return((long)(p - buf + 5));
    }
}


// parse buffer in which lies adif-file completely
int parse_qso(char *buf, long start, long end) {
    long dlen;
    char *tag = buf + start, *len, *gtsign, *nexttag;
    char lenfield[LENFIELDSZ];
    char tagname[TAGNAMESZ];
    char tagvalue[TAGVALSZ];
    int err = 0;

    tag = index(tag, '<');
    memset(&adi, 0, sizeof(ADI));

    do {
        memset(lenfield, 0, LENFIELDSZ);
        memset(tagname, 0, TAGNAMESZ);
        memset(tagvalue, 0, TAGVALSZ);
        //            <call:6>DO1MHR<
        //            ^tag ^len     ^nexttag
        //                   ^gtsign
        get_positions(tag, &len, &gtsign, &nexttag);
        strncpy(lenfield, len + 1, (gtsign - len - 1));
        dlen = atoi(lenfield);
        strncpy(tagname, tag + 1, len - tag - 1);
        strncpy(tagvalue, gtsign + 1, dlen);
        fill_adif_record(tagname, tagvalue);
        tag = nexttag;
    } while ( tag < (buf + end - 5) );

    err = write_adif_record();
    if ( err > 0 ) {
        QMessageBox msgbox( QMessageBox::Warning, "Write Adif Error",
               QString("Can not write sqlite3 output File sqlite3.txt(%1)").arg(errno) );
        msgbox.exec();
    }
    return(1);
}


// find starting positions of tags and values
void get_positions(char *tag, char **len, char **gtsign, char **nexttag) {
    //            <call:6>DO1MHR<
    //            ^tag ^len     ^nexttag
    //                   ^gtsign
    *len     = index(tag, ':');
    *gtsign  = index(tag, '>');
    *nexttag = index(tag+1, '<');
}


// fill ADI Structure for later writing into db
void fill_adif_record(char *f, char *v)  {
    for ( int i = 0; i < NRADIFIELDS; i++ ) {
        if ( !strcasecmp(adif_fields[i], f) ) {
            strcpy((char*)adi.f[i], v);
            //           printf(">%s<>%s<\n", f, v);
            return;
        }
    }
//    messages(MSG_WARN, "App does not know adif field", f, 0);
    QMessageBox msgbox( QMessageBox::Critical, "Unknown ADIF TAG!",
           QString("App does not know adif field:%1, value:%2, PLEASE INFORM THE DEVTEAM of this APP!").arg(f).arg(v));
    msgbox.exec();
}


// seems to be missing in c++
void strtoupper(char *s) {
    while((*s=toupper(*s)))
        s++;
}


// now we write realy into db
int write_adif_record() {
    int icall = 0, idate = 0, itime = 0, iband = 0, imode = 0;
    char ssql[2048];

    icall = find_adif_field("call");
    idate = find_adif_field("qso_date");
    itime = find_adif_field("time_on");
    iband = find_adif_field("band");
    imode = find_adif_field("mode");

    // if time is not correct format (6 chars) fill up with zeroes
    if (strlen((char*)adi.f[itime]) == 4)
        strcat((char*)adi.f[itime], "00");

    if ( icall < 0 || idate < 0 || itime < 0 ) {
        QMessageBox msgbox( QMessageBox::Information, QString("Missing Info!"),
                           QString("record discarded, call/date/time missing!") );
        msgbox.exec();
        return(0);
    }

    strtoupper((char*)adi.f[icall]);

    doquery("\n\nbegin transaction;\n");
    sprintf(ssql, "insert into qso(call,date,time,band,mode) values('%s','%s','%s','%s','%s');\n",
            adi.f[icall], adi.f[idate], adi.f[itime], adi.f[iband], adi.f[imode]);
    doquery(ssql);
    doquery("update tnr set nr = last_insert_rowid();\n");
    for ( int i = 0; i < NRADIFIELDS; i++ ) {
        if ( adi.f[i] != NULL && strlen((char*)adi.f[i]) > 0) {
            if ( strncmp((char*)adif_fields[i], "time_", 5) == 0 && strlen((char*)adi.f[i]) < 6) {
                strncat((char*)adi.f[i], "0000",6 - strlen((char*)adi.f[i]));
            }
            sprintf(ssql, "insert into qsod values((select nr from tnr limit 1),'%s','%s');\n", adif_fields[i], adi.f[i]);
            doquery(ssql);
        }
    }

// TODO? Fill additional Fields???

    doquery("commit transaction;\n");

    return(0);
}


// find a field called char *what in already filled ADI Structure
int find_adif_field(const char *what) {
    for ( int i = 0; i < NRADIFIELDS; i++ )
        if ( !strcasecmp(adif_fields[i], what) )
            return(i);
    return(-1);
}



