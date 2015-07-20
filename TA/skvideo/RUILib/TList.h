/*
	* The MIT License (MIT)
	* Copyright (c) 2015 SK PLANET. All Rights Reserved.
	*
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	*
	* The above copyright notice and this permission notice shall be included in
	* all copies or substantial portions of the Software.
	*
	* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	* THE SOFTWARE.
*/

#ifndef _TLIST_H
#define _TLIST_H

template <class CollectionNode> class TList
{
public:
	TList();
	~TList();
private:
	CollectionNode* m_pHeader;
	int             m_iCount;
public:
	bool            Init();
	bool            FreeAll();
	bool            IsEmpty();
	bool            InsertAfter(CollectionNode* pPrevNode, CollectionNode* pNewNode);
	bool            InsertBefore(CollectionNode* pNextNode, CollectionNode* pNewNode);
	bool            InsertAtHead(CollectionNode* pNode);
	bool            InsertAtTail(CollectionNode* pNode);
	bool            RemoveHead();
	bool            RemoveTail();
	bool			Remove(CollectionNode* pNode);
	bool            Unlink(CollectionNode* pNode);
	CollectionNode* GetPrev(CollectionNode *pNode) const;
	CollectionNode* GetNext(CollectionNode *pNode) const;
	CollectionNode* GetFirst() const;
	CollectionNode* GetLast() const;
	CollectionNode* GetHeader();
	CollectionNode* GetAt(int iIndex);
	int             GetCount() const { return m_iCount; }
};

template <class CollectionNode> TList<CollectionNode>::TList()
{
    m_pHeader = NULL;
    Init();
}

template <class CollectionNode> TList<CollectionNode>::~TList()
{
    FreeAll();
    delete m_pHeader;
    m_pHeader = NULL;
}

template <class CollectionNode> bool TList<CollectionNode>::Init()
{
    if (m_pHeader != NULL)
        return false;

    m_pHeader = new CollectionNode;

    if (m_pHeader == NULL)
        return false;

    m_pHeader->pPrev = m_pHeader;
    m_pHeader->pNext = m_pHeader;

    m_iCount = 0;

    return true;
}

template <class CollectionNode> bool TList<CollectionNode>::FreeAll()
{
    if (m_pHeader == NULL)
        return false;

    CollectionNode* pNode;
    CollectionNode* pNextNode;

    for (pNode = m_pHeader->pNext; pNode != m_pHeader; )
    {
        pNextNode = pNode->pNext;
        Unlink(pNode);
        delete pNode;
        pNode = pNextNode;
    }

#ifdef _DEBUG
	if (m_iCount != 0)
		ASSERT(FALSE);
#endif

    m_iCount = 0;

    return true;
}

template <class CollectionNode> bool TList<CollectionNode>::IsEmpty(void)
{
    return (m_pHeader->pNext == m_pHeader);
}

template <class CollectionNode> bool TList<CollectionNode>::InsertAfter(CollectionNode* pPrevNode, CollectionNode* pNewNode)
{
    if (pPrevNode == NULL)
        return false;

    if (pNewNode == NULL)
        return false;

    pPrevNode->pNext->pPrev = pNewNode;
    pNewNode->pNext = pPrevNode->pNext;

    pPrevNode->pNext = pNewNode;
    pNewNode->pPrev = pPrevNode;

    m_iCount++;

    return true;
}

template <class CollectionNode> bool TList<CollectionNode>::InsertBefore(CollectionNode* pNextNode, CollectionNode* pNewNode)
{
    if (pNextNode == NULL)
        return false;

    if (pNewNode == NULL)
        return false;

    pNextNode->pPrev->pNext = pNewNode;
    pNewNode->pPrev = pNextNode->pPrev;

    pNextNode->pPrev = pNewNode;
    pNewNode->pNext = pNextNode;

    m_iCount++;

    return true;
}

template <class CollectionNode> bool TList<CollectionNode>::InsertAtHead(CollectionNode* pNewNode)
{
    if (pNewNode == NULL)
        return false;

    m_pHeader->pNext->pPrev = pNewNode;
    pNewNode->pNext = m_pHeader->pNext;

    m_pHeader->pNext = pNewNode;
    pNewNode->pPrev = m_pHeader;

    m_iCount++;

    return true;
}

template <class CollectionNode> bool TList<CollectionNode>::InsertAtTail(CollectionNode* pNewNode)
{
    if (pNewNode == NULL)
        return false;

    m_pHeader->pPrev->pNext = pNewNode;
    pNewNode->pPrev = m_pHeader->pPrev;

    m_pHeader->pPrev = pNewNode;
    pNewNode->pNext = m_pHeader;

    m_iCount++;

    return true;
}

template <class CollectionNode> bool TList<CollectionNode>::RemoveHead()
{
    if (m_pHeader == NULL)
        return false;

    CollectionNode* pNode = m_pHeader->pNext;
    if (pNode != m_pHeader)
    {
        Unlink(pNode);
        delete pNode;
//		m_iCount--; // [neuromos] !모든 소스에 반영할 것!

		return true;
    }

	return false;
}

template <class CollectionNode> bool TList<CollectionNode>::RemoveTail()
{
    if (m_pHeader == NULL)
        return false;

    CollectionNode* pNode = m_pHeader->pPrev;
    if (pNode != m_pHeader)
    {
        Unlink(pNode);
        delete pNode;
//		m_iCount--; // [neuromos] !모든 소스에 반영할 것!

		return true;
    }

	return false;
}

template <class CollectionNode> bool TList<CollectionNode>::Remove(CollectionNode* pNode)
{
	if (pNode == NULL || pNode == m_pHeader)
		return false;

	Unlink(pNode);
	delete pNode;

//	m_iCount--; // [neuromos] !모든 소스에 반영할 것!

	return true;
}

template <class CollectionNode> bool TList<CollectionNode>::Unlink(CollectionNode* pNode)
{
    if (pNode == NULL)
        return false;

	if( pNode->pPrev != NULL )
	{
		pNode->pPrev->pNext = pNode->pNext;
	}

	if( pNode->pNext != NULL )
	{
		pNode->pNext->pPrev = pNode->pPrev;
	}

	m_iCount--; // [neuromos] !모든 소스에 반영할 것!

    return true;
}

template <class CollectionNode> CollectionNode* TList<CollectionNode>::GetFirst() const
{
    if (m_pHeader == NULL)
        return NULL;

    if (m_pHeader->pNext != m_pHeader)
    {
        return m_pHeader->pNext;
    }

    return NULL;
}

template <class CollectionNode> CollectionNode* TList<CollectionNode>::GetLast() const
{
    if (m_pHeader == NULL)
        return NULL;

    if (m_pHeader->pPrev != m_pHeader)
    {
        return m_pHeader->pPrev;
    }

    return NULL;
}

template <class CollectionNode> CollectionNode* TList<CollectionNode>::GetPrev(CollectionNode* pNode) const
{
    if (m_pHeader == NULL)
        return NULL;

    if (pNode == NULL)
        return NULL;

    if (pNode->pPrev != m_pHeader)
    {
        return pNode->pPrev;
    }

    return NULL;
}

template <class CollectionNode> CollectionNode* TList<CollectionNode>::GetNext(CollectionNode* pNode) const
{
    if (m_pHeader == NULL)
        return NULL;

    if (pNode == NULL)
        return NULL;

    if (pNode->pNext != m_pHeader)
    {
        return pNode->pNext;
    }

    return NULL;
}

template <class CollectionNode> CollectionNode* TList<CollectionNode>::GetHeader()
{
    return m_pHeader;
}

template <class CollectionNode> CollectionNode* TList<CollectionNode>::GetAt(int iIndex)
{
    if (m_pHeader == NULL)
        return NULL;

    if (iIndex >= GetCount())
        return NULL;

    CollectionNode* pNode;
    int             i;

    pNode = m_pHeader->pNext;

    for (i = 0; i < iIndex; i++)
    {
        pNode = pNode->pNext;

        if (pNode == m_pHeader)
            return NULL;
    }

    if (pNode != m_pHeader)
    {
        return pNode;
    }

    return NULL;
}

#endif
