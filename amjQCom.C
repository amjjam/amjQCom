#include "amjQCom.H"

#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <deque>

#include <amjCom/amjCom.H>

namespace amjQCom {

  // ==========================================================
  // Internal data structure
  // ==========================================================

  struct StatusEntry {
    QDateTime time;
    QString stateDescription;
    QString errorText;

    int count= 1;
    QDateTime firstTime;
    QDateTime lastTime;
  };

  // ==========================================================
  // Helpers
  // ==========================================================

  static QString formatErrorText(const StatusEntry &e) {
    if(e.count <= 1)
      return e.errorText;

    double dt= e.firstTime.msecsTo(e.lastTime) / 1000.0;

    if(dt <= 0.0)
      return e.errorText + QString(" (x%1)").arg(e.count);

    double rate= e.count / dt;

    return QString("%1 (x%2 in %3 s, %4 Hz)")
      .arg(e.errorText)
      .arg(e.count)
      .arg(dt, 0, 'f', 2)
      .arg(rate, 0, 'f', 1);
  }

  static bool sameEntry(const StatusEntry &a, const StatusEntry &b) {
    return a.stateDescription == b.stateDescription
           && a.errorText == b.errorText;
  }

  // ==========================================================
  // Impl
  // ==========================================================

  class ClientStatus::Impl {
  public:
    Impl(ClientStatus *parent): q(parent) {

      indicator= new QWidget(q);
      indicator->setFixedSize(12, 12);
      indicator->setStyleSheet("background-color: gray; border-radius: 6px;");

      label= new QLabel("NoState", q);

      auto layout= new QHBoxLayout(q);
      layout->addWidget(indicator);
      layout->addWidget(label);
      layout->addStretch();
      layout->setContentsMargins(2, 2, 2, 2);

      // =========================
      // Popup window
      // =========================

      popup= new QFrame(nullptr);
      popup->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
      popup->setFrameShape(QFrame::Box);
      popup->setMinimumSize(300, 150);

      // =========================
      // Header
      // =========================

      header= new QWidget(popup);
      auto h= new QHBoxLayout(header);
      h->setContentsMargins(4, 2, 4, 2);

      title= new QLabel("Status history", header);

      pinButton= new QToolButton(header);
      pinButton->setCheckable(true);
      pinButton->setAutoRaise(true);
      pinButton->setCursor(Qt::PointingHandCursor);
      pinButton->setFixedSize(20, 20);
      pinButton->setStyleSheet("background: transparent;");

      // Load icons from resources
      pinOn= QIcon(":/pin.png");
      pinOff= QIcon(":/pin_off.png");

      updatePinIcon(false);

      h->addWidget(title);
      h->addStretch();
      h->addWidget(pinButton);

      // =========================
      // Table
      // =========================

      table= new QTableWidget(popup);
      table->setColumnCount(3);
      table->setHorizontalHeaderLabels({ "Time", "State", "Error" });
      table->horizontalHeader()->setStretchLastSection(true);

      // =========================
      // Layout
      // =========================

      auto v= new QVBoxLayout(popup);
      v->addWidget(header);
      v->addWidget(table);

      // =========================
      // Pin behavior
      // =========================

      QObject::connect(pinButton, &QToolButton::toggled, [this](bool on) {
        pinned= on;
        updatePinIcon(on);

        popup->setWindowFlag(Qt::WindowStaysOnTopHint, on);
        popup->show();

        // Visual feedback
        if(pinned)
          header->setStyleSheet("background-color: #e0f0ff;");
        else
          header->setStyleSheet("");
      });
    }

    // ------------------------------------------------------

    void updatePinIcon(bool pinnedState) {
      if(pinnedState) {
        pinButton->setIcon(pinOn);
        pinButton->setToolTip("Unpin window");
      } else {
        pinButton->setIcon(pinOff);
        pinButton->setToolTip("Pin window");
      }
    }

    // ------------------------------------------------------

    ClientStatus *q;

    QLabel *label;
    QWidget *indicator;

    QFrame *popup;
    QWidget *header;
    QLabel *title;
    QToolButton *pinButton;
    QTableWidget *table;

    QIcon pinOn;
    QIcon pinOff;

    std::deque<StatusEntry> history;
    int maxEntries= 1000;
    int lastRow= -1;

    StatusEntry pending;
    bool hasPending= false;

    bool pinned= false;

    const int maxBurstMs= 5000;

    // ------------------------------------------------------

    void updateUI(const StatusEntry &e, const amjCom::Status &status) {
      label->setText(e.stateDescription);

      QString color= "gray";

      if(status.error() != amjCom::NoError)
        color= "red";
      else if(status.state() == amjCom::Connected)
        color= "green";
      else if(status.state() == amjCom::Connecting
              || status.state() == amjCom::Resolving)
        color= "orange";

      indicator->setStyleSheet(
        QString("background-color: %1; border-radius: 6px;").arg(color));

      q->setToolTip(e.time.toString("HH:mm:ss") + " | " + e.stateDescription
                    + " | " + e.errorText);
    }

    // ------------------------------------------------------

    void addEntry(const StatusEntry &e) {
      history.push_back(e);

      int row= table->rowCount();
      table->insertRow(row);
      lastRow= row;

      table->setItem(
        row, 0, new QTableWidgetItem(e.time.toString("yyyy-MM-dd HH:mm:ss")));

      table->setItem(row, 1, new QTableWidgetItem(e.stateDescription));
      table->setItem(row, 2, new QTableWidgetItem(formatErrorText(e)));

      if(e.errorText != "None") {
        for(int c= 0; c < 3; c++)
          table->item(row, c)->setForeground(Qt::red);
      }

      if((int)history.size() > maxEntries) {
        history.pop_front();
        table->removeRow(0);
        lastRow--;
      }

      table->scrollToBottom();
    }

    // ------------------------------------------------------

    void updateLast(const StatusEntry &e) {
      if(lastRow < 0)
        return;

      history.back()= e;
      table->item(lastRow, 2)->setText(formatErrorText(e));
    }

    // ------------------------------------------------------

    void handleStatus(const amjCom::Status &status) {
      StatusEntry e;

      e.time= QDateTime::currentDateTime();
      e.firstTime= e.time;
      e.lastTime= e.time;

      e.stateDescription= QString::fromStdString(status.statedescription());

      QString errorDesc
        = QString::fromStdString(amjCom::errordescriptions[status.error()]);

      QString errorMsg= QString::fromStdString(status.errormessage());

      if(status.error() == amjCom::NoError)
        e.errorText= "None";
      else if(errorMsg.isEmpty())
        e.errorText= errorDesc;
      else
        e.errorText= errorDesc + ": " + errorMsg;

      updateUI(e, status);

      if(!hasPending) {
        pending= e;
        hasPending= true;
        addEntry(pending);
        return;
      }

      if(sameEntry(e, pending)
         && pending.firstTime.msecsTo(e.time) < maxBurstMs) {
        pending.count++;
        pending.lastTime= e.time;
        updateLast(pending);
      } else {
        pending= e;
        pending.count= 1;
        pending.firstTime= e.time;
        pending.lastTime= e.time;
        addEntry(pending);
      }
    }
  };

  // ==========================================================
  // Public API
  // ==========================================================

  ClientStatus::ClientStatus(QWidget *parent): QWidget(parent) {
    _impl= new Impl(this);

    static bool registered= false;
    if(!registered) {
      qRegisterMetaType<amjCom::Status>("amjCom::Status");
      registered= true;
    }

    connect(
      this, &ClientStatus::statusReceived, this,
      [this](amjCom::Status status) { _impl->handleStatus(status); },
      Qt::QueuedConnection);

    qApp->installEventFilter(this);
  }

  ClientStatus::~ClientStatus() { delete _impl; }

  void ClientStatus::setMaxHistory(int n) { _impl->maxEntries= n; }

  void ClientStatus::pushStatus(const amjCom::Status status) {
    emit statusReceived(status);
  }
  //  new QLabel(title, parent));
  // ------------------------------------------------------

  void ClientStatus::mousePressEvent(QMouseEvent *) {
    if(_impl->popup->isVisible() && !_impl->pinned) {
      _impl->popup->hide();
      return;
    }

    QPoint pos= mapToGlobal(QPoint(0, height()));
    _impl->popup->move(pos);

    if(_impl->popup->size().isEmpty())
      _impl->popup->resize(width() * 2, 300);

    _impl->popup->show();
    _impl->popup->raise();
    _impl->popup->activateWindow();
  }

  // ------------------------------------------------------

  bool ClientStatus::eventFilter(QObject *, QEvent *event) {
    if(!_impl->popup->isVisible() || _impl->pinned)
      return false;

    if(event->type() == QEvent::MouseButtonPress) {
      QWidget *w= QApplication::widgetAt(QCursor::pos());

      if(!w || !_impl->popup->isAncestorOf(w)) {
        _impl->popup->hide();
      }
    }

    return false;
  }

}// namespace amjQCom
