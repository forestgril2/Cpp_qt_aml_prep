// my_view_model.cpp
// Implementation of MyViewModel (C++/Qt MVVM pattern)

#include "my_view_model.h"

MyViewModel::MyViewModel(QObject* parent)
    : QObject(parent), m_counter(0) {}

int MyViewModel::counter() const {
    return m_counter;
}

void MyViewModel::setCounter(int value) {
    if (m_counter != value) {
        m_counter = value;
        emit counterChanged(m_counter);
    }
}

void MyViewModel::increment() {
    setCounter(m_counter + 1);
}

void MyViewModel::reset() {
    setCounter(0);
} 