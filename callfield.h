#ifndef CALLFIELD_H
#define CALLFIELD_H

#include <QLineEdit>
#include <QMainWindow>
#include <QObject>
#include <QKeyEvent>
//#include <QQuickItem>
#include <QSharedDataPointer>
#include <QWidget>

class CallField : public QLineEdit
{
    Q_OBJECT

public:
    CallField(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *event);
};


#endif // CALLFIELD_H
