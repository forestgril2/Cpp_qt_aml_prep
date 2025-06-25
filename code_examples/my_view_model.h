// my_view_model.h
// Example of a ViewModel class in C++/Qt (MVVM pattern)
#pragma once

#include <QObject>

class MyViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY counterChanged)

public:
    explicit MyViewModel(QObject* parent = nullptr);

    int counter() const;
    void setCounter(int value);

signals:
    void counterChanged(int newValue);

public slots:
    void increment();
    void reset();

private:
    int m_counter = 0;
}; 