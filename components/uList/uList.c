/**
 * @file        uList.h
 * @author      Mohammad Sadequr Rahman <sadequr.rahman.rabby@gmail.com>
 * @date        10 May 2021
 * @brief       A Simple and easy to use c list library.
 *
 * 
 *
 * @copyright   Copyright (C) 2021 Mohammad Sadequr Rahman. All rights reserved.
 */

#include "include/uList.h"


uList_t* uList_createList(void)
{
    uList_t *lst = (uList_t*) calloc(1,sizeof(uList_t));
    if (lst)
    {
        lst->head = NULL;
        lst->tail = NULL;
        lst->numItems = 0;
    }
    return lst;
}

uNode_t* uList_createNode( void *_val, uint16_t _len, uNode_alloc_t _f)
{
    uNode_t * node = (uNode_t*) calloc(1, sizeof(uNode_t));
    if(node)
    {
        node->flag = _f;
        if(node->flag == NODE_SELF_ALLOC)
        {
            node->value = (void*) calloc(1, _len);
            memcpy(node->value,_val,_len);
        }   
        else
            node->value = _val;
        node->len = _len;
        node->nextNode = NULL;
        node->previousNode = NULL;
        node->index = 0;
    }
    return node;
}

uNode_t* uList_findByValue(uList_t * _list, void *_val)
{
    uNode_t *cHead = _list->head;
    while (cHead)
    {
        if(memcmp(_val, cHead->value, cHead->len) == 0)
            return cHead;
        cHead = cHead->previousNode;
    }
    return NULL;
}

uNode_t* uList_findByIdx(uList_t * _list, uint16_t _idx)
{
    uNode_t *cHead = _list->head;
    while (cHead)
    {
        if(cHead->index == _idx)
            return cHead;
        cHead = cHead->previousNode;
    }
    return NULL;
}
uErr_t uList_append(uList_t * _list, uNode_t * _node)
{
    if(!_list)
    {
        return uErr_list_invalid;
    }
    if(!_node)
    {
        return uErr_node_invalid;
    }
    if(_list->tail == NULL)
        _list->tail = _node;
    if(_list->head != NULL)
        _list->head->nextNode = _node;
    _node->previousNode = _list->head;
    _list->head = _node;
    _node->index = _list->numItems;
    _list->numItems++;
    return uErr_fail;
}

uErr_t uList_insertAt(uList_t * _list, uint16_t _idx, uNode_t * _node)
{
    if(!_list)
        return uErr_list_invalid;
    if(!_node)
        return uErr_node_invalid;
    uNode_t *tail = _list->tail;
    uint8_t isFound = 0;
    while (tail)
    {
        if(isFound == 1)
            tail->index += 1;
        if(tail->index == _idx)
        {   
            isFound = 1;
            _node->previousNode = tail->previousNode;
            if(tail->previousNode != NULL)
                tail->previousNode->nextNode = _node;
            else
                _list->tail = _node;
            tail->previousNode = _node;
            _node->nextNode = tail;
            _node->index = _idx;
            tail->index = _idx+1;
            _list->numItems++;
        }
        tail = tail->nextNode;
    }
    return (isFound == 0 ? uErr_index_invalid : uErr_ok );
}

uErr_t uList_remove(uList_t * _list)
{
    if(!_list)
        return uErr_list_invalid;
    while (_list->head)
    {
        uNode_t *node = _list->head;
        _list->head = _list->head->previousNode;
        if(node->flag == NODE_SELF_ALLOC)
            free(node->value);
        free(node);
    }
    free(_list);
    return uErr_ok;
    
}

uErr_t uList_removeByIdx(uList_t * _list, uint16_t _idx)
{
    uint8_t isFound = 0;
    uNode_t *toBefreed = NULL;
    uNode_t *node = _list->tail;
    while (node)
    {
        if (isFound == 1)
        {
            node->index -= 1;
        }
        if(node->index == _idx && isFound == 0)
        {
            isFound = 1;
            if(node->previousNode != NULL)
                node->previousNode->nextNode = node->nextNode;
            else
                _list->tail = NULL;
            if(node->nextNode != NULL)
                node->nextNode->previousNode = node->previousNode;
            else
                _list->head = node->previousNode;
            if(node->flag == NODE_SELF_ALLOC)
                free(node->value);
            toBefreed = node;
            _list->numItems--;
        }
        node = node->nextNode;
    }
    if(isFound==1)
        free(toBefreed);
    return ( isFound == 0 ? uErr_fail : uErr_ok);
}

uErr_t uList_removeByValue(uList_t * _list, void *_val)
{
    uint8_t isFound = 0;
    uNode_t *toBefreed = NULL;
    uNode_t *node = _list->tail;
    while (node)
    {
        if (isFound == 1)
        {
            node->index -= 1;
        }
        if(memcmp(_val, node->value, node->len) == 0 && isFound == 0)
        {
            isFound = 1;
            if(node->previousNode != NULL)
                node->previousNode->nextNode = node->nextNode;
            else
                _list->tail = NULL;
            if(node->nextNode != NULL)
                node->nextNode->previousNode = node->previousNode;
            else
                _list->head = node->previousNode;
            if(node->flag == NODE_SELF_ALLOC)
                free(node->value);
            toBefreed = node;
            _list->numItems--;
        }
        node = node->nextNode;
    }
    if(isFound==1)
        free(toBefreed);
    return ( isFound == 0 ? uErr_fail : uErr_ok);
}

uint16_t uList_getSize(uList_t * _list)
{
    return _list->numItems;
}

uErr_t uList_delete(uList_t *_list)
{
    if (_list)
    {
        uNode_t *node = _list->tail;
        while (node)
        {
            uNode_t *tmp = node;
            node = node->nextNode;
            free(tmp);
        }    
        return uErr_ok;
    }
    
    return  uErr_fail;
}