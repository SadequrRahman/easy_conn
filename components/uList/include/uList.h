/**
 * @file        uList.h
 * @author      Mohammad Sadequr Rahman <sadequr.rahman.rabby@gmail.com>
 * @date        7 May 2021
 * @brief       A Simple and easy to use c list library.
 *
 * 
 * @remarks     This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *              GNU Lesser General Public License for more details.
 *
 *              You should have received a copy of the GNU Lesser General Public License
 *              along with this program; if not, write to the Free Software
 *              Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * @copyright   Copyright (C) 2021 Mohammad Sadequr Rahman. All rights reserved.
 */

#ifndef INC_ULIST_H_
#define INC_ULIST_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef enum{
   NODE_SELF_ALLOC,
   NODE_NON_SELF_ALLOC
}uNode_alloc_t;

typedef enum
{
   uErr_ok = 0,
   uErr_fail,
   uErr_list_invalid,
   uErr_node_invalid,
   uErr_index_invalid,
   max_num_err
}uErr_t;

typedef struct node
{
   struct node * nextNode;
   struct node * previousNode;
   void * value;
   uint16_t len;
   uint16_t index;
   uint8_t flag;
}uNode_t;


typedef struct 
{
   uNode_t * head;
   uNode_t * tail;
   uint16_t  numItems;
}uList_t;

#define ITERATE_LIST(list, value, len, code)                \
do {                                            \
    if (list) {                                 \
       uNode_t *node = list->tail;              \
       while (node)                             \
      {  \
         value = node->value;                   \
         len = node->len;\
         code;          \
         node = node->nextNode;                \
      }                                         \
    }                                           \
} while (0)


uList_t* uList_createList(void);
uNode_t* uList_createNode( void * _val, uint16_t _len, uNode_alloc_t _f);
uNode_t* uList_findByValue(uList_t * _list, void * _val);
uNode_t* uList_findByIdx(uList_t * _list, uint16_t _idx);
uErr_t uList_append(uList_t * _list, uNode_t * _node);
uErr_t uList_insertAt(uList_t * _list, uint16_t _idx, uNode_t * _node);
uErr_t uList_remove(uList_t * _list);
uErr_t uList_removeByIdx(uList_t * _list, uint16_t _idx);
uErr_t uList_removeByValue(uList_t * _list, void * _val);
uint16_t uList_getSize(uList_t * _list); 
uErr_t uList_delete(uList_t *_list);

#endif //INC_ULIST_H_