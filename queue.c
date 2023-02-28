#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *current, *next;
    list_for_each_entry_safe (current, next, l, list) {
        list_del(&current->list);
        q_release_element(current);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *rm = list_first_entry(head, element_t, list);
    if (sp != NULL) {
        strncpy(sp, rm->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&rm->list);
    return rm;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *rm = list_last_entry(head, element_t, list);
    if (sp != NULL) {
        strncpy(sp, rm->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&rm->list);
    return rm;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *node;
    int n = 0;
    list_for_each (node, head)
        n++;
    return n;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || !head->next)
        return false;
    struct list_head *fast = head->next, *slow = head->next;
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    element_t *current, *forward;
    struct list_head *index = head->next;
    char *del = NULL;
    while (index != head) {
        del = list_entry(index, element_t, list)->value;
        bool flag = 0;
        for (current = list_entry(index, element_t, list),
            forward = list_entry(current->list.next, element_t, list);
             &current->list != head; current = forward,
            forward = list_entry(forward->list.next, element_t, list)) {
            if (del && !strcmp(current->value, del) &&
                &current->list != index) {
                list_del_init(&current->list);
                q_release_element(current);
                flag = 1;
            }
        };
        if (del && flag) {
            struct list_head *temp = index;
            index = index->next;
            list_del(temp);
            q_release_element(list_entry(temp, element_t, list));
            flag = 0;
        } else
            index = index->next;
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node;
    for (node = head->next; node != head && node->next != head;
         node = node->next) {
        struct list_head *next = node->next;
        list_del_init(node);
        list_add(node, next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *prev = head->prev, *current = head, *next = NULL;
    while (next != head) {
        next = current->next;
        current->next = prev;
        current->prev = next;
        prev = current;
        current = next;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || k <= 1)
        return;
    struct list_head *node, *safe, *temp = head;
    LIST_HEAD(pseudo);
    int count = 0;
    list_for_each_safe (node, safe, head) {
        count++;
        if (count == k) {
            list_cut_position(&pseudo, temp, node);
            q_reverse(&pseudo);
            list_splice_init(&pseudo, temp);
            count = 0;
            temp = safe->prev;
        }
    }
}

/* Merge two list into one queue */
struct list_head *merge_two_list(struct list_head *l1, struct list_head *l2)
{
    struct list_head *temp = NULL;
    struct list_head **indirect = &temp;
    for (struct list_head **node = NULL; l1 && l2; *node = (*node)->next) {
        element_t *e1 = list_entry(l1, element_t, list);
        element_t *e2 = list_entry(l2, element_t, list);
        if (strcmp(e1->value, e2->value) < 0)
            node = &l1;
        else
            node = &l2;
        *indirect = *node;
        indirect = &(*indirect)->next;
    }
    *indirect = (struct list_head *) ((u_int64_t) l1 | (u_int64_t) l2);
    return temp;
}

/* Divide the queue and sort every element */
struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *fast = head, *slow = head;
    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    fast = slow;
    slow->prev->next = NULL;
    struct list_head *l1 = mergesort(head);
    struct list_head *l2 = mergesort(fast);
    return merge_two_list(l1, l2);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    head->prev->next = NULL;
    head->next = mergesort(head->next);
    struct list_head *current = head, *next = head->next;
    while (next) {
        next->prev = current;
        current = next;
        next = next->next;
    }
    current->next = head;
    head->prev = current;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    element_t *current, *forward;
    char *max = NULL;
    int total = 0, count = 0;
    for (current = list_entry(head->prev, element_t, list),
        forward = list_entry(current->list.prev, element_t, list);
         &current->list != head; current = forward,
        forward = list_entry(forward->list.prev, element_t, list)) {
        total++;
        if (!max || strcmp(current->value, max) > 0) {
            max = current->value;
        }
        if (max && strcmp(current->value, max) < 0) {
            list_del(&current->list);
            q_release_element(current);
            count++;
        }
    }
    return total - count;
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head)) {
        return 0;
    }
    if (list_is_singular(head)) {
        return list_entry(head->next, queue_contex_t, chain)->size;
    }
    queue_contex_t *merged_list = list_entry(head->next, queue_contex_t, chain);
    struct list_head *node = NULL, *safe = NULL;
    list_for_each_safe (node, safe, head) {
        if (node == head->next) {
            continue;
        }
        queue_contex_t *temp = list_entry(node, queue_contex_t, chain);
        list_splice_init(temp->q, merged_list->q);
    }
    q_sort(merged_list->q);
    return merged_list->size;
}
