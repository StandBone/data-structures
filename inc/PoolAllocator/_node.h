#pragma once

template <class T>
union _node
{
    _node* next;
    T value;
};
