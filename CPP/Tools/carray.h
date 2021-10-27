#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "cquicksort.h"

#define sizeofa(_a)	((sizeof(_a) / sizeof(*(_a))))

//-----------------------------------------------------------------------------

template < class DATA_T > 
class CArray : public CQuickSort < DATA_T > {

	// ----------------------------------------

	class CArrayInfo {
		public:
			DATA_T* buffer;
			DATA_T	none;
			size_t	length;
			size_t	pos;
			size_t	mode;
			bool	wrap;

		public:

			CArrayInfo() : buffer(nullptr), length(0), pos(0), mode(0), wrap(false) {
				memset(&none, 0, sizeof(none));
			}

			inline size_t Length (void) { return length; }
		};

	protected:
		CArrayInfo		m_info;

		// ----------------------------------------

	public:
		class Iterator {
			private:
				DATA_T *		m_start;
				DATA_T *		m_end;
				DATA_T *		m_current;

			public:
				explicit Iterator () : m_start (nullptr), m_end (nullptr), m_current (nullptr) {}

				Iterator (CArray& a) : m_start (a.Start ()), m_end (a.End () + 1), m_current (nullptr) {}

				operator bool() const { 
					return m_current != nullptr; 
				}

				DATA_T* operator*() const { 
					return m_current; 
				}

				Iterator& operator++() { 
					++m_current;
					return *this;
				}

				Iterator& operator--() { 
					--m_current;
					return *this;
				}

				Iterator& Start (void) {
					m_current = m_start;
					return *this;
				}

				Iterator& End (void) {
					m_current = m_end;
					return *this;
				}

				bool operator== (Iterator& other) {
					return m_current == other.m_current;
				}

				bool operator!= (Iterator& other) {
					return m_current != other.m_current;
				}

		};

		// ----------------------------------------

		CArray() { 
			Init (); 
			}
		
		explicit CArray(const size_t nLength) { 
			Init (); 
			Create (nLength);
			}
		
		CArray(CArray const& other) {
			Init (); 
			Copy (other);
			}
		
		CArray (CArray&& other) {
			Move (other);
		}

		explicit CArray(DATA_T const * data, size_t dataLength) {
			Init();
			Create(dataLength);
			memcpy(m_info.buffer, data, sizeof(DATA_T) * dataLength);
		}

		CArray(std::initializer_list<DATA_T> data) {
			Init();
			Create(data.size ());
			memcpy(m_info.buffer, data.begin (), sizeof(DATA_T) * data.size ());
		}

		~CArray() {
			Destroy (); 
		}
		
		Iterator begin() {
			return Iterator (*this).Start();
		}

		Iterator end() {
			return Iterator (*this).End();
		}

		// ----------------------------------------

		void Init (void) { 
			m_info.buffer = reinterpret_cast<DATA_T *> (nullptr); 
			m_info.length = 0;
			m_info.pos = 0;
			m_info.mode = 0;
			m_info.wrap = false;
			memset (&m_info.none, 0, sizeof (m_info.none));
			}

		// ----------------------------------------

		void Clear (uint8_t filler = 0, size_t count = 0xffffffff) { 
			if (m_info.buffer) 
				memset (m_info.buffer, filler, sizeof (DATA_T) * ((count < m_info.length) ? count : m_info.length)); 
			}

		// ----------------------------------------

		void Fill (DATA_T filler, size_t count = 0xffffffff) {
			if (m_info.buffer) {
				if (count > m_info.length)
					count = m_info.length;
				for (DATA_T* bufP = Buffer(); count; count--, bufP++)
					*bufP = filler;
			}
		}

		// ----------------------------------------

		inline bool IsIndex (size_t i) { 
			return (m_info.buffer != nullptr) && (i < m_info.length); 
		}
		
		// ----------------------------------------

		inline bool IsElement (DATA_T* elem, bool bDiligent = false) {
			if (!m_info.buffer || (elem < m_info.buffer) || (elem >= m_info.buffer + m_info.length))
				return false;	// no buffer or element out of buffer
			if (bDiligent) {
				size_t i = static_cast<size_t> (reinterpret_cast<uint8_t*> (elem) - reinterpret_cast<uint8_t*> (m_info.buffer));
				if (i % sizeof (DATA_T))	
					return false;	// elem in the buffer, but not properly aligned
			}
			return true;
		}

		// ----------------------------------------

		inline size_t Index (DATA_T* elem) { 
			return size_t (elem - m_info.buffer); 
		}

		// ----------------------------------------

		inline DATA_T* Pointer (size_t i) { 
			return m_info.buffer + i; 
		}

		// ----------------------------------------

		void Destroy (void) { 
			if (m_info.buffer) {
				if (!m_info.mode)
					delete[] m_info.buffer;
				Init ();
			}
		}
			
		// ----------------------------------------

		DATA_T *Create (size_t length) {
			if (m_info.length != length) {
				Destroy ();
				if ((m_info.buffer = new DATA_T [length]))
					m_info.length = length;
			}
			return m_info.buffer;
		}
			
		// ----------------------------------------

		inline DATA_T* Buffer (size_t i = 0) const { 
			return m_info.buffer + i; 
		}
		
		// ----------------------------------------

		void SetBuffer (DATA_T *buffer, size_t mode = 0, size_t length = 0xffffffff) {
			if (m_info.buffer != buffer) {
				if (!(m_info.buffer = buffer))
					Init ();
				else {
					m_info.length = length;
					m_info.mode = mode;
				}
			}
		}
			
		// ----------------------------------------

		DATA_T* Resize (size_t length, bool bCopy = true) {
			if (m_info.mode == 2)
				return m_info.buffer;
			if (!m_info.buffer)
				return Create (length);
			DATA_T* p;
			try {
				p = new DATA_T [length];
			}
			catch(...) {
				return m_info.buffer;
			}
			if (bCopy) {
				memcpy (p, m_info.buffer, ((length > m_info.length) ? m_info.length : length) * sizeof (DATA_T)); 
				Clear (); // hack to avoid d'tors
			}
			m_info.length = length;
			m_info.pos %= length;
			delete[] m_info.buffer;
			return m_info.buffer = p;
		}

		// ----------------------------------------

		inline const size_t Length (void) const { 
			return m_info.length; 
		}

		// ----------------------------------------

		inline DATA_T* Current (void) { 
			return m_info.buffer ? m_info.buffer + m_info.pos : nullptr; 
		}

		// ----------------------------------------

		inline size_t Size (void) { 
			return m_info.length * sizeof (DATA_T); 
		}

		// ----------------------------------------

		inline DATA_T& operator[] (const size_t i) {
			return m_info.buffer [i]; 
		}

		// ----------------------------------------

		inline DATA_T* operator* () const { 
			return m_info.buffer; 
		}

		// ----------------------------------------

		inline CArray<DATA_T>& operator= (CArray<DATA_T> const & source) {
			return _Copy (source.Buffer (), source.Length ()); 
		}

		// ----------------------------------------

		inline CArray<DATA_T>& operator= (CArray<DATA_T>&& source) {
			return Move (source);
		}

		// ----------------------------------------

		inline CArray<DATA_T>& operator= (std::initializer_list<DATA_T> data) {
			Init();
			Create(data.size());
			memcpy(m_info.buffer, data.begin(), sizeof(DATA_T) * data.size());
			return *this;
		}

		// ----------------------------------------

		inline DATA_T& operator= (DATA_T* source) { 
			memcpy (m_info.buffer, source, m_info.length * sizeof (DATA_T)); 
			return m_info.buffer [0];
		}

		// ----------------------------------------

		inline CArray<DATA_T>& Copy (CArray<DATA_T> const & source, size_t offset = 0) { 
			return _Copy(source.Buffer(), source.Length(), offset);
		}

		// ----------------------------------------

		CArray<DATA_T>& _Copy(DATA_T const* source, size_t length, size_t offset = 0) {
			if ((m_info.buffer && (m_info.length >= length + offset)) || Resize(length + offset, false))
				memcpy(m_info.buffer + offset, source, ((m_info.length - offset < length) ? m_info.length - offset : length) * sizeof(DATA_T));
			return *this;
		}

		// ----------------------------------------

		CArray<DATA_T>& Move (CArray<DATA_T>& source) {
			Destroy ();
			m_info.buffer = source.m_info.buffer;
			m_info.length = source.m_info.length;
			m_info.pos = source.m_info.pos;
			m_info.mode = source.m_info.mode;
			m_info.wrap = source.m_info.wrap;
			source.m_info.buffer = nullptr;
			source.Destroy ();
			return *this;
		}

		// ----------------------------------------

		inline DATA_T operator+ (CArray<DATA_T>& source) { 
			CArray<DATA_T> a (*this);
			a += source;
			return a;
		}

		// ----------------------------------------

		inline DATA_T& operator+= (CArray<DATA_T>& source) { 
			size_t offset = m_info.length;
			if (m_info.buffer) 
				Resize (m_info.length + source.m_info.length);
			return Copy (source, offset);
		}

		// ----------------------------------------

		inline bool operator== (CArray<DATA_T>& other) { 
			return (m_info.length == other.m_info.length) && !(m_info.length && memcmp (m_info.buffer, other.m_info.buffer)); 
		}

		// ----------------------------------------

		inline bool operator!= (CArray<DATA_T>& other) { 
			return (m_info.length != other.m_info.length) || (m_info.length && memcmp (m_info.buffer, other.m_info.buffer)); 
		}

		// ----------------------------------------

		inline DATA_T* Start (void) { return m_info.buffer; }

		// ----------------------------------------

		inline DATA_T* End (void) { return (m_info.buffer && m_info.length) ? m_info.buffer + m_info.length - 1 : nullptr; }

		// ----------------------------------------

		inline DATA_T* operator++ (void) { 
			if (!m_info.buffer)
				return nullptr;
			if (m_info.pos < m_info.length - 1)
				m_info.pos++;
			else if (m_info.wrap) 
				m_info.pos = 0;
			else
				return nullptr;
			return m_info.buffer + m_info.pos;
		}

		// ----------------------------------------

		inline DATA_T* operator-- (void) { 
			if (!m_info.buffer)
				return nullptr;
			if (m_info.pos > 0)
				m_info.pos--;
			else if (m_info.wrap)
				m_info.pos = m_info.length - 1;
			else
				return nullptr;
			return m_info.buffer + m_info.pos;
		}

		// ----------------------------------------

		inline DATA_T* operator+ (size_t i) { 
			return m_info.buffer ? m_info.buffer + i : nullptr; 
		}

		// ----------------------------------------

		inline DATA_T* operator- (size_t i) { return m_info.buffer ? m_info.buffer - i : nullptr; }

		// ----------------------------------------

		CArray<DATA_T>& ShareBuffer (CArray<DATA_T>& child) {
			memcpy (&child.m_info, &m_info, sizeof (m_info));
			if (!child.m_info.mode)
				child.m_info.mode = 1;
			return child;
			}

		// ----------------------------------------

		inline bool operator! () { return m_info.buffer == nullptr; }

		// ----------------------------------------

		inline size_t Pos (void) { return m_info.pos; }

		// ----------------------------------------

		inline void Pos (size_t pos) { m_info.pos = pos % m_info.length; }

		// ----------------------------------------
#if 0
		size_t Read (CFile& cf, size_t nCount = 0, size_t nOffset = 0, bool bCompressed = 0) { 
			if (!m_info.buffer)
				return -1;
			if (nOffset >= m_info.length)
				return -1;
			if (!nCount)
				nCount = m_info.length - nOffset;
			else if (nCount > m_info.length - nOffset)
				nCount = m_info.length - nOffset;
			return cf.Read (m_info.buffer + nOffset, sizeof (DATA_T), nCount, bCompressed);
			}

		// ----------------------------------------

		size_t Write (CFile& cf, size_t nCount = 0, size_t nOffset = 0, size_t bCompressed = 0) { 
			if (!m_info.buffer)
				return -1;
			if (nOffset >= m_info.length)
				return -1;
			if (!nCount)
				nCount = m_info.length - nOffset;
			else if (nCount > m_info.length - nOffset)
				nCount = m_info.length - nOffset;
			return cf.Write (m_info.buffer + nOffset, sizeof (DATA_T), nCount, bCompressed);
			}
#endif

		// ----------------------------------------

		inline void SetWrap (bool wrap) { m_info.wrap = wrap; }

		// ----------------------------------------

		inline void SortAscending (size_t left = 0, size_t right = -1) { 
			if (m_info.buffer) 
				CQuickSort<DATA_T>::SortAscending (m_info.buffer, left, (right >= 0) ? right : m_info.length - 1); 
				}

		// ----------------------------------------

		inline void SortDescending (size_t left = 0, size_t right = -1) {
			if (m_info.buffer) 
				CQuickSort<DATA_T>::SortDescending (m_info.buffer, left, (right >= 0) ? right : m_info.length - 1);
			}

		// ----------------------------------------

		inline void SortAscending (CQuickSort<DATA_T>::tComparator compare, size_t left = 0, size_t right = -1) {
			if (m_info.buffer) 
				CQuickSort<DATA_T>::SortAscending (m_info.buffer, left, (right >= 0) ? right : m_info.length - 1, compare);
			}

		// ----------------------------------------

		inline void SortDescending (CQuickSort<DATA_T>::tComparator compare, size_t left = 0, size_t right = -1) {
			if (m_info.buffer) 
				CQuickSort<DATA_T>::SortDescending (m_info.buffer, left, (right >= 0) ? right : m_info.length - 1, compare);
			}

		// ----------------------------------------

		inline size_t BinSearch (DATA_T key, size_t left = 0, size_t right = -1) {
			return m_info.buffer ? CQuickSort<DATA_T>::BinSearch (m_info.buffer, left, (right >= 0) ? right : m_info.length - 1, key) : -1;
			}
	};

//-----------------------------------------------------------------------------

inline size_t operator- (char* v, CArray<char>& a) { return a.Index (v); }
inline size_t operator- (uint8_t* v, CArray<uint8_t>& a) { return a.Index (v); }
inline size_t operator- (int16_t* v, CArray<int16_t>& a) { return a.Index (v); }
inline size_t operator- (uint16_t* v, CArray<uint16_t>& a) { return a.Index (v); }
inline size_t operator- (uint32_t* v, CArray<uint32_t>& a) { return a.Index (v); }
inline size_t operator- (size_t* v, CArray<size_t>& a) { return a.Index (v); }

//-----------------------------------------------------------------------------

class CCharArray : public CArray<char> {
	public:
		inline char* operator= (const char* source) { 
			size_t l = size_t (strlen (source) + 1);
			if ((l > this->m_info.length) && !this->Resize (this->m_info.length + l))
				return nullptr;
			memcpy (this->m_info.buffer, source, l);
			return this->m_info.buffer;
		}
};

//-----------------------------------------------------------------------------

class CByteArray : public CArray<uint8_t> {};
class CShortArray : public CArray<int16_t> {};
class CUShortArray : public CArray<uint16_t> {};
class CIntArray : public CArray<int32_t> {};
class CUIntArray : public CArray<uint32_t> {};
class CSizeArray : public CArray<size_t> {};
class CFloatArray : public CArray<float> {};

//-----------------------------------------------------------------------------

template < class DATA_T, size_t length > 
class CStaticArray : public CArray < DATA_T > {

	class CStaticArrayData {
		public:
			DATA_T		buffer [length];
			};

	protected:
		CStaticArrayData	m_info;

	public:
		CStaticArray () { Create (length); }

		DATA_T *Create (size_t _length) {
			this->SetBuffer (m_info.buffer, 2, _length); 
			return m_info.buffer;
			}
		void Destroy (void) { }
	};

//-----------------------------------------------------------------------------
