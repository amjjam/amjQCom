#ifndef _AMJQCOM_H_
#define _AMJQCOM_H_

#include <QMetaType>
#include <QWidget>

// Forward declare your Status class
namespace amjCom {
  class Status;
}

#include <amjCom/amjCom.H>

// Metatype declaration
Q_DECLARE_METATYPE(amjCom::Status)

namespace amjQCom {

  class ClientStatus: public QWidget {
    Q_OBJECT

  public:
    explicit ClientStatus(QWidget *parent= nullptr);
    virtual ~ClientStatus();

    void pushStatus(const amjCom::Status status);
    void setMaxHistory(int n);

  signals:
    void statusReceived(const amjCom::Status status);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    class Impl;
    Impl *_impl;
  };

}// namespace amjQCom

#endif

// #ifndef _AMJQCOM_H_
// #define _AMJQCOM_H_

// #include <QMetaType>
// #include <QWidget>

// // Forward declare your Status class
// namespace amjCom {
//   class Status;
// }

// #include <amjCom/amjCom.H>

// // Metatype declaration (safe here, keeps Qt out of amjCom)
// Q_DECLARE_METATYPE(amjCom::Status)

// namespace amjQCom {

//   class ClientStatus: public QWidget {
//     Q_OBJECT

//   public:
//     explicit ClientStatus(QWidget *parent= nullptr);
//     virtual ~ClientStatus();

//     // Thread-safe entry point
//     void pushStatus(const amjCom::Status &status);

//     // Limit number of history entries
//     void setMaxHistory(int n);

//   signals:
//     void statusReceived(const amjCom::Status &status);

//   protected:
//     void mousePressEvent(QMouseEvent *event) override;
//     bool eventFilter(QObject *obj, QEvent *event) override;

//   private:
//     class Impl;
//     Impl *_impl;
//   };

// }// namespace amjQCom

// #endif
