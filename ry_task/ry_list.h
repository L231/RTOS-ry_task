#ifndef __RY_LIST_H__
	#define	__RY_LIST_H__

#include "ry_type.h"





/**
 * �����������ʼ��
 *
 **/
ry_inline void ry_list_init(ry_list_t *l)
{
	l->next = l->prev = l;
}





/**
 * ������ָ������ڵ�ĺ�������½ڵ�
 *
 *  l ----> ln       l ----> ln       l              ln       l               ln
 *          |   ==>          |   ==>  |              |   ==>  |               |
 *  n <-----+        n <---->+        +----> n <---->+        +<----> n <---->+
 *
 **/
ry_inline void ry_list_insert_after(ry_list_t *l, ry_list_t *n)
{
	l->next->prev = n;         /* ����ǰ�ڵ�ĺ�һ���ڵ��ǰ������ */
	n->next       = l->next;   /* �������ŵ�ǰ�ڵ�ĺ�һ���ڵ� */
	l->next       = n;         /* ����ǰ�ڵ�ĺ������� */
	n->prev       = l;         /* ǰ�����ŵ�ǰ�ڵ� */
}





/**
 * ������ָ������ڵ��ǰ������½ڵ�
 *
 * lp <---- l       lp <---- l       lp              l       lp               l
 *  |          ==>   |          ==>   |              |  ==>   |               |
 *  +-----> n        +<----> n        +<----> n <----+        +<----> n <---->+
 *
 **/
ry_inline void ry_list_insert_before(ry_list_t *l, ry_list_t *n)
{
	l->prev->next = n;
	n->prev       = l->prev;
	l->prev       = n;
	n->next       = l;
}





/**
 * ������ɾ�������еĽڵ�
 *
 **/
ry_inline void ry_list_remove(ry_list_t *n)
{
	n->prev->next = n->next;
	n->next->prev = n->prev;
	n->next       = n->prev = n;
}













#endif
