#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <type_traits>
#include <cstddef>
#include <utility>
#include <iostream>
#include "carray.h"

#define USE_DATA_PTR	0
#if USE_DATA_PTR
#	define DATA_PTR(data)	(data)
#	define DATA_VALUE(data)	(*(data))
#else
#	define DATA_PTR(data)	(&data)
#	define DATA_VALUE(data)	(data)
#endif

//-----------------------------------------------------------------------------

template < class DATA_T >
class CList {

	private:
		class CNode;

		//----------------------------------------

		class CNodePtr {
			public:
				CNode* m_ptr;

				explicit CNodePtr () : m_ptr (nullptr) {}
/*
				CNodePtr(const CNodePtr& other) {
					m_ptr = other.m_ptr;
				}
*/
				explicit CNodePtr(CNode * ptr) : m_ptr(ptr) {}

				CNodePtr& operator= (CNodePtr other) {
					m_ptr = other.m_ptr;
					return *this;
				}

				CNodePtr& operator= (CNode * p) {
					m_ptr = p;
					return *this;
				}

				void operator++ (int) {
					*this = m_ptr->m_next;
				}

				void operator-- (int) {
					*this = m_ptr->m_prev;
				}

				CNodePtr operator+ (int n) {
					CNodePtr ptr = *this;
					for (; n; n--)
						ptr++;
					return ptr;
				}

				CNodePtr operator- (int n) {
					CNodePtr ptr = *this;
					for (; n; n--)
						ptr--;
					return ptr;
				}

				CNode* operator->() const {
					return m_ptr;
				}

				bool operator== (const CNodePtr& other) {
					return m_ptr == other.m_ptr;
				}

				bool operator!= (const CNodePtr& other) {
					return m_ptr != other.m_ptr;
				}

				operator bool() {
					return (m_ptr != nullptr);
				}
			};

		//----------------------------------------

		class CNode {
			public:
				CNodePtr	m_prev;
				CNodePtr	m_next;
#if USE_DATA_PTR
				DATA_T *	m_data;
#else
				DATA_T		m_data;
#endif

#if USE_DATA_PTR
				explicit CNode() : m_prev (nullptr), m_next (nullptr), m_data (nullptr) {}
#else
				explicit CNode () : m_prev (nullptr), m_next (nullptr) {}
#endif

				~CNode() {
#if USE_DATA_PTR
					if (m_data) {
						delete m_data;
						m_data = nullptr;
					}
#endif
				}

		};

		//----------------------------------------

		class Iterator {

			private:
				CNodePtr 	m_start;
				CNodePtr 	m_end;
				CNodePtr 	m_current;
				size_t		m_index;

			public:
				explicit Iterator() {}

				Iterator(CList< DATA_T >& l) : m_start(l.First ()), m_end(l.Last () + 1), m_index (0) {}

				operator bool() const { return m_current != nullptr; }

				//DATA_T* operator*() const { return &m_current->m_data; }
				// return the current value and the index of the value in the list
				constexpr std::pair<size_t, DATA_T&> operator*() const { 
					return std::pair<size_t, DATA_T&> {m_index, DATA_VALUE (m_current->m_data) };
				}

				Iterator& operator++() {
					m_current++;
					m_index++;
					return *this;
				}

				Iterator& operator--() {
					m_current--;
					return *this;
				}

				bool operator== (Iterator& other) {
					return m_current == other.m_current;
				}

				bool operator!= (Iterator& other) {
					return m_current != other.m_current;
				}

				//Iterator& Start(void) {
				constexpr Iterator& Start(void) {
					m_index = 0;
					m_current = m_start;
					return *this;
				}

				Iterator& End(void) {
					m_current = m_end;
					return *this;
				}
			};

	// ----------------------------------------

	private:
		CNode		m_head;
		CNode		m_tail;
#if USE_DATA_PTR
		DATA_T*		m_none;
#else
		DATA_T		m_none;
#endif
		CNodePtr	m_headPtr;
		CNodePtr 	m_tailPtr;
		CNodePtr 	m_nullPtr;
		size_t		m_length;
		bool		m_result;

	public:
		void Destroy(void) {
			if (m_headPtr) {
				CNodePtr p, n;
				for (n = m_headPtr + 1; n != m_tailPtr; ) {
					p = n;
					n++;
					if (p.m_ptr) {
						delete p.m_ptr;
						p.m_ptr = nullptr;
					}
				}
			}
#if USE_DATA_PTR
			if (m_none) {
				delete m_none;
				m_none = nullptr;
			}
#endif
			Init();
		}

		CList<DATA_T>& Copy(const CList<DATA_T> & other) {
			Destroy();
			CNodePtr p = other.m_headPtr;
			for (p++; p != other.m_tailPtr; p++)
				Insert (-1, DATA_VALUE (p.m_ptr->m_data));
			return *this;
		}

		inline void Init(void) {
			m_headPtr = &m_head;
			m_tailPtr = &m_tail;
			m_headPtr->m_prev = m_tailPtr->m_next = nullptr;
			m_headPtr->m_next = &m_tail;
			m_tailPtr->m_prev = &m_head;
#if USE_DATA_PTR
			m_none = new DATA_T;
#endif
			m_length = 0;
		}

		Iterator begin() {
			return Iterator(*this).Start();
		}

		Iterator end() {
			return Iterator(*this).End();
		}

		inline CNodePtr Head(void) {
			return m_headPtr;
		}

		inline CNodePtr Tail(void) {
			return m_tailPtr;
		}

		inline CNodePtr First(void) {
			return m_head.m_next;
		}

		inline CNodePtr Last(void) {
			return m_tail.m_prev;
		}

		inline size_t Length(void) {
			return m_length;
		}

		inline const bool Empty(void) const {
			return m_length == 0;
		}

		inline DATA_T& operator[] (const size_t i) {
			CNodePtr p = NodePtr(int (i), m_headPtr + 1, m_tailPtr - 1);
			if (p) {
				m_result = true;
				return DATA_VALUE (p->m_data);
			}
			m_result = true;
			return DATA_VALUE (m_none);
		}
#if 0
		inline DATA_T& operator[] (DATA_T& d) {
			int i = Find(d);
			if (i < 0)
				return DATA_VALUE (m_none);
			CNodePtr p = NodePtr(int(i), m_headPtr + 1, m_tailPtr - 1);
			return p ? DATA_VALUE (p->m_data) : DATA_VALUE (m_none);
		}
#endif
		inline CList<DATA_T>& operator= (CList<DATA_T> const& other) {
			Copy(other);
			return *this;
		}

		inline CList<DATA_T>& operator= (CList<DATA_T>&& other) noexcept {
			return Move (other);
		}

		inline CList<DATA_T>& operator= (std::initializer_list<DATA_T> data) {
			Destroy();
			for (auto const& d : data)
				Append(d);
			return *this;
		}

		explicit CList() : 
#if USE_DATA_PTR
			m_none (nullptr),
#endif
			m_length (0), m_result (true) {
			Init();
		}

		CList(CList<DATA_T> const& other) :
#if USE_DATA_PTR
			m_none (nullptr),
#endif
			m_length (0), m_result (true) {
			Copy(other);
		}
		
		CList(CList<DATA_T>&& other) noexcept :
#if USE_DATA_PTR
			m_none (nullptr),
#endif
			m_length (0), m_result (true) {
			Init ();
			Move(other);
		}

		explicit CList(DATA_T& data) : 
#if USE_DATA_PTR
			m_none(nullptr),
#endif
			m_length(0), m_result(true) {
#if USE_DATA_PTR
			m_none = new DATA_T;
#endif
			Append(data);
		}

		explicit CList(CArray<DATA_T>& data) : 
#if USE_DATA_PTR
			m_none (nullptr),
#endif
			m_length (0), m_result (true) {
			Init ();
			for (auto const& v : data)
				Append(*v);
		}

		CList(std::initializer_list<DATA_T> data) : 
#if USE_DATA_PTR
			m_none (nullptr),
#endif
			m_length (0), m_result (true) {
			Init ();
			for (auto const& d : data)
				Append(d);
		}

		~CList() {
			Destroy();
		}

//-----------------------------------------------------------------------------

private:
CNodePtr NodePtr(int i, CNodePtr first, CNodePtr last) {
	if (i == 0)
		return first;
	if (i == -1)
		return last;
	CNodePtr p;
	if (i > 0) {
		int32_t h = int (m_length - i);
		if (h < i)
			i = -h;
	}
	if (i >= 0) {
		p = first;
		for (; i && (p != m_tailPtr); i--, p++)
			;
	}
	else
		for (p = last; ++i && (p != m_headPtr); p = p->m_prev)
			;
	return i ? m_nullPtr : p;

}

//-----------------------------------------------------------------------------

public:
DATA_T* Add(int i) {
	CNodePtr p = NodePtr(i, m_headPtr + 1, m_tailPtr);
	if (!p)
		return nullptr;
	CNode* n = new CNode;
	if (!n)
		return nullptr;
#if USE_DATA_PTR
	if (!(n->m_data = new DATA_T)) {
		delete n;
		return nullptr;
	}
#else
#endif
	n->m_prev = p->m_prev;
	n->m_prev->m_next = n;
	n->m_next = p;
	p->m_prev = n;
	m_length++;
#if USE_DATA_PTR
	return n->m_data;
#else
	return &n->m_data;
#endif
}

//-----------------------------------------------------------------------------

public:
bool Insert (int i, DATA_T data) {
	DATA_T * dataPtr = Add (i);
	if (!dataPtr)
		return false;
	*dataPtr = data;
	return true;
}

//-----------------------------------------------------------------------------

public:
auto Pop(int i) {
	m_result = false;
	if (!m_length)
		return DATA_VALUE (m_none);

	CNodePtr nodePtr = NodePtr(i, m_headPtr + 1, m_tailPtr - 1);
	if (!nodePtr)
		return DATA_VALUE (m_none);
	CNode* p = nodePtr.m_ptr;
	DATA_T data = DATA_VALUE (p->m_data);
	p->m_prev->m_next = p->m_next;
	p->m_next->m_prev = p->m_prev;
	delete p;
	m_length--;
	m_result = true;
	return data;
}

//-----------------------------------------------------------------------------

#if 0
public:
auto Pop(DATA_T data) {
	for (auto [i, d] : *this)
		if (d == data)
			return Pop(i);
	return DATA_VALUE (m_none);
}
#endif

//-----------------------------------------------------------------------------
// move the other list to the end of this list
// will leave other list empty

public:
CList< DATA_T >& operator+= (CList< DATA_T > other) {
	if (other.Empty())
		return *this;
#if 1
	for (auto [i, d] : other)
		Insert (-1, d);
#else
	CNode* s = other.m_head.m_next.m_ptr;
	CNode* e;
	if (Empty()) {
		m_headPtr.m_ptr->m_next = s;
		s->m_prev = m_headPtr;
	}
	else {
		e = m_tailPtr.m_ptr->m_prev.m_ptr;
		e->m_next = s;
		s->m_prev = e;
	}
	e = other.m_tail.m_prev.m_ptr;
	e->m_next = &m_tail;
	m_tail.m_prev = e;
	m_tailPtr = &m_tail;
	other.Init ();
#endif
	return *this;
}

//-----------------------------------------------------------------------------
// move the other list to the end of this list
// will leave other list empty

public:
CList< DATA_T >& Move (CList< DATA_T >& other) {
	Destroy ();
	if (other.Empty ())
		return *this;
	CNode* s = other.m_head.m_next.m_ptr;
	CNode* e;
	if (Empty ()) {
		m_headPtr.m_ptr->m_next = s;
		s->m_prev = m_headPtr;
	}
	else {
		e = m_tailPtr.m_ptr->m_prev.m_ptr;
		e->m_next = s;
		s->m_prev = e;
	}
	e = other.m_tail.m_prev.m_ptr;
	e->m_next = m_tailPtr;
	m_tailPtr->m_prev = e;
	m_length = other.m_length;
	other.Init ();
	return *this;
}

//-----------------------------------------------------------------------------

public:
CList< DATA_T > operator+ (const CList< DATA_T >& other) {
	CList< DATA_T > l;
	if (Empty())
		return l = other;
	if (other.Empty())
		return l = *this;
	l = *this;
	l += other;
	return l;
}

//-----------------------------------------------------------------------------

public:
int Find(DATA_T data) {
#if 1
	for (const auto [i, p] : *this)
		if (p == data)
			return int (i);
#else
	int i = 0;
	for (CNodePtr p = m_headPtr->m_next; p != m_tailPtr; p++, i++)
		if (*p.m_ptr->m_data == data)
			return int (i);
#endif
	return -1;
}

//-----------------------------------------------------------------------------

public:
CList< DATA_T > Splice(size_t from, size_t to) {
	CNode* start = NodePtr(int (from));
	CNode* end = NodePtr(int (to ? to : m_length - 1));
	CList< DATA_T > l;
	if ((start != nullptr) && (end != nullptr)) {
		for (CNode* i = start; i != end; i = i->next) {
			CNode& n = l.Append(new CNode);
			*n.m_data = *i.m_data;
		}
	}
	return l;
}

//-----------------------------------------------------------------------------

public:
inline bool Append(DATA_T data) {
	return Insert(-1, data);
}

//-----------------------------------------------------------------------------

public:
inline bool Remove(DATA_T data) {
	int i = Find(data);
	if (i < 0)
		return (m_result = false);
	Pop(size_t(i));
	return (m_result = true);
}

//-----------------------------------------------------------------------------

public:
inline bool Result(void) {
	return m_result;
}

//-----------------------------------------------------------------------------

};
