#include "notification.h"
#include <QIcon>
#include <QPixmap>

#include <QDebug>

Notification::Notification()
{
   //notificator->setPixmap(QIcon(":/img/kfilebox.png").pixmap(48,48));
}

void Notification::send(QString msg)
{
    system(QString("notify-send KFileBox '"+msg+"'").toStdString().c_str());
    /*if (conf->getShowNotifications()==true){
        notificator= new KNotification("notify",KNotification::Persistent);
        notificator->setTitle("Kfilebox");
        //notificator->setPixmap(QIcon(":/img/white/kfileboxapp.png").pixmap(48,48));
        notificator->setText(msg);
        //notificator->event(notificator->eventId(),"Kfilebox",msg,QPixmap(":/img/white/kfileboxapp.png"));
        notificator->sendEvent();
    }*/
}
