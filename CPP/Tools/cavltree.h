#pragma once

#include <stdexcept>
#include "string.h"

    template < class KEY_T, class DATA_T >
    class CAvlTree {
	    public:

            typedef int(*tComparator) (KEY_T const &, KEY_T const &);

            //----------------------------------------

		    class CNode;

		    typedef CNode* tNodePtr;

		    class CNode {
			    public:
				    KEY_T		key;
				    DATA_T		data;
				    tNodePtr	left;
				    tNodePtr	right;
				    char		balance;

				    CNode() : left(nullptr), right(nullptr), balance(0) {}

				    CNode(KEY_T key, DATA_T data) : key (key), data (data), left(nullptr), right(nullptr), balance(0) {}
			    };

		    //----------------------------------------

		    struct tAvlInfo {
			    tNodePtr	root;
			    tNodePtr	current;
			    KEY_T	    key;
                tComparator compare;
			    bool	    isDuplicate;
			    bool        branchChanged;

                tAvlInfo () : root (nullptr), current (nullptr), compare (nullptr), isDuplicate (false), branchChanged (false) {}
            };

		    typedef bool (* tNodeProcessor) (DATA_T const &);

	    private:

		    tAvlInfo	m_info;


		//----------------------------------------

	public:

		CAvlTree () {
            memset (&m_info, 0, sizeof (m_info));
        }

		~CAvlTree () { 
			Delete (); 
		}

        inline void SetComparator(tComparator compare) {
            m_info.compare = compare;
        }

//-----------------------------------------------------------------------------

#define AVL_OVERFLOW   1
#define AVL_BALANCED   0
#define AVL_UNDERFLOW  3

///-----------------------------------------------------------------------------

public:
DATA_T* Find(KEY_T key)
{
    for (tNodePtr node = m_info.root; node; ) {
        switch (m_info.compare(key, node->key)) {
            case -1:
                node = node->left;
                break;
            case 1:
                node = node->right;
                break;
            default:
                m_info.current = node;
                return &node->data;
        }
    }
    return nullptr;
}

//-----------------------------------------------------------------------------

private:    
tNodePtr AllocNode(void)
{
    if (m_info.current = new CNode()) {
        m_info.branchChanged = true;
        return m_info.current;
    }
    return nullptr;
}

//-----------------------------------------------------------------------------

public:
bool InsertNode(tNodePtr& root)
{
    tNodePtr p1, p2, r = root;
    if (!r)
        return ((root = AllocNode()) != nullptr);

    switch (m_info.compare (m_info.key, r->key)) {
        case -1:
            if (!InsertNode(r->left))
                return false;
            if (m_info.branchChanged) {
                switch (r->balance) {
                case AVL_UNDERFLOW:
                    p1 = r->left;
                    if (p1->balance == AVL_UNDERFLOW) {  // single LL rotation
                        r->left = p1->right;
                        p1->right = r;
                        r->balance = AVL_BALANCED;
                        r = p1;
                    }
                    else { // double LR rotation
                        p2 = p1->right;
                        p1->right = p2->left;
                        p2->left = p1;
                        r->left = p2->right;
                        p2->right = r;
                        char b = p2->balance;
                        r->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
                        p1->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
                        r = p2;
                    }
                    r->balance = AVL_BALANCED;
                    m_info.branchChanged = false;
                    break;

                case AVL_BALANCED:
                    r->balance = AVL_UNDERFLOW;
                    break;

                case AVL_OVERFLOW:
                    r->balance = AVL_BALANCED;
                    m_info.branchChanged = false;
                    break;
                }
            }
            break;

        case 0:
            m_info.isDuplicate = true;
            m_info.current = r;
            break;

        case 1:
            if (!InsertNode(r->right))
                return false;
            if (m_info.branchChanged) {
                switch (r->balance) {
                case AVL_UNDERFLOW:
                    r->balance = AVL_BALANCED;
                    m_info.branchChanged = false;
                    break;

                case AVL_BALANCED:
                    r->balance = AVL_OVERFLOW;
                    break;

                case AVL_OVERFLOW:
                    tNodePtr p1 = r->right;
                    if (p1->balance == AVL_OVERFLOW) { // single RR rotation
                        r->right = p1->left;
                        p1->left = r;
                        r->balance = AVL_BALANCED;
                        r = p1;
                    }
                    else { // double RL rotation
                        tNodePtr p2 = p1->left;
                        p1->left = p2->right;
                        p2->right = p1;
                        r->right = p2->left;
                        p2->left = r;
                        char b = p2->balance;
                        r->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
                        p1->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
                        r = p2;
                    }
                    r->balance = AVL_BALANCED;
                    m_info.branchChanged = false;
                }
            }
        break;
    }
    root = r;
    return true;
}

//-----------------------------------------------------------------------------

public:
bool Insert(KEY_T key, DATA_T data)
{
    m_info.key = key;
    m_info.branchChanged = m_info.isDuplicate = false;
    if (!InsertNode(m_info.root))
        return false;
    if (!m_info.isDuplicate) {
        m_info.current->key = key;
        m_info.current->data = data;
    }
    return true;
}

//-----------------------------------------------------------------------------

private:
void BalanceLShrink(tNodePtr& root, bool& branchShrunk)
{
    tNodePtr r = root;
    switch (r->balance) {
    case AVL_UNDERFLOW:
        r->balance = AVL_BALANCED;
        break;

    case AVL_BALANCED:
        r->balance = AVL_OVERFLOW;
        branchShrunk = false;
        break;

    case AVL_OVERFLOW:
        tNodePtr p1 = r->right;
        char b = p1->balance;
        if (b != AVL_UNDERFLOW) { // single RR rotation
            r->right = p1->left;
            p1->left = r;
            if (b)
                r->balance = p1->balance = AVL_BALANCED;
            else {
                r->balance = AVL_OVERFLOW;
                p1->balance = AVL_UNDERFLOW;
                branchShrunk = false;
            }
            r = p1;
        }
        else { // double RL rotation
            tNodePtr p2 = p1->left;
            b = p2->balance;
            p1->left = p2->right;
            p2->right = p1;
            r->right = p2->left;
            p2->left = r;
            r->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
            p1->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
            r = p2;
            r->balance = AVL_BALANCED;
        }
    }
    root = r;
}

//-----------------------------------------------------------------------------

private:
void BalanceRShrink(tNodePtr& root, bool& branchShrunk)
{
    tNodePtr r = *root;
    switch (r->balance) {
        case AVL_OVERFLOW:
            r->balance = AVL_BALANCED;
            break;

        case AVL_BALANCED:
            r->balance = AVL_UNDERFLOW;
            branchShrunk = false;
            break;

        case AVL_UNDERFLOW:
            tNodePtr p1 = r->left;
            char b = p1->balance;
            if (b != AVL_OVERFLOW) { // single LL rotation
                r->left = p1->right;
                p1->right = r;
                if (b)
                    r->balance = p1->balance = AVL_BALANCED;
                else {
                    r->balance = AVL_UNDERFLOW;
                    p1->balance = AVL_OVERFLOW;
                    branchShrunk = false;
                }
                r = p1;
            }
            else { // double LR rotation
                tNodePtr p2 = p1->right;
                b = p2->balance;
                p1->right = p2->left;
                p2->left = p1;
                r->left = p2->right;
                p2->right = r;
                r->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
                p1->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
                r = p2;
                r->balance = AVL_BALANCED;
            }
    }
    root = r;
}

//-----------------------------------------------------------------------------
//Free sucht das groesste Element im linken Unterbaum des auszufuegenden  
// Elements. Es nimmt die Stelle des auszufuegenden Elements im Baum ein. Dazu
// werden die Datenpointer der beiden Baumknoten *root und *p vertauscht.     
// Danach kann inDelete *root geloescht werden. Seine Stelle im-Baum  
// wird vom Knoten root->left eingenommen. Welcher Knoten zu loeschen ist,    
// wird vonFree in p vermerkt.                                            

private:
void UnlinkNode(tNodePtr& root, tNodePtr& delNode, bool& branchShrunk)
{
    tNodePtr r = root;
    if (r->right) {
        Free(r->right, delNode, branchShrunk);
        if (branchShrunk)
            BalanceRShrink(r, branchShrunk);
    }
    else {
        tNodePtr d = delNode;
        DATA_T h = r->data;
        r->data = d->data;
        d->data = h;
        d = r;
        r = r->left;
        branchShrunk = true;
        delNode = d;
    }
    root = r;
}

//-----------------------------------------------------------------------------

private:
bool RemoveNode(tNodePtr& root, bool& branchChanged)
{
    if (!root)
        branchChanged = false;
    else {
        tNodePtr r = root;
        if (m_info.key < r->key) {
            if (!RemoveNode(r->left))
                return false;
            if (branchChanged)
                BalanceLShrink(r, branchChanged);
        }
        else if (m_info.key > r->key) {
            if (!RemoveNode(r->right))
                return false;
            if (branchChanged)
                BalanceRShrink(r, branchChanged);
        }
        else {
            tNodePtr d = r;
            if (!r->right) {
                r = r->left;
                branchChanged = true;
            }
            else if (!r->left) {
                r = r->right;
                branchChanged = true;
            }
            else {
                UnlinkNode(d->left, d, branchChanged);
                if (branchChanged)
                    BalanceLShrink(r, branchChanged);
            }
            m_info.data = d->data;
            delete d;
        }
        root = r;
    }
    return true;
}

//-----------------------------------------------------------------------------

public:
bool Remove(KEY_T key)
{
    if (!m_info.root)
        return false;
    m_info.key = key;
    m_info.branchChanged = false;
    return Delete(m_info.root, m_info.branchChanged);
}

//-----------------------------------------------------------------------------

private:
void DeleteNodes(tNodePtr& root)
{
    if (root) {
        DeleteNodes(root->left);
        DeleteNodes(root->right);
        delete root;
    }
}

//-----------------------------------------------------------------------------

public:
void Delete(void)
{
    DeleteNodes(m_info.root);
}

//-----------------------------------------------------------------------------

private:
bool WalkNodes(tNodePtr root, tNodeProcessor processNode)
{
    if (root) {
        if (!WalkNodes(root->left, processNode))
            return false;
        if (!processNode(root->data))
            return false;
        if (!WalkNodes(root->right, processNode))
            return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

public:
bool Walk(tNodeProcessor processNode)
{
    return WalkNodes(m_info.root, processNode);
} /*AvlWalk*/

//-----------------------------------------------------------------------------

public:
DATA_T* Min(void)
{
    if (!m_info.root)
        return nullptr;
    tNodePtr p = m_info.root;
    for (; p->left; p = p->left)
        ;
    return &p->data;
}

//-----------------------------------------------------------------------------

public:
DATA_T* Max(void)
{
    if (!m_info.root)
        return nullptr;
    tNodePtr p = m_info.root;
    for (; p->right; p = p->right)
        ;
    return &p->data;
}

//-----------------------------------------------------------------------------

private:
void ExtractMin(tNodePtr& root, bool& branchChanged)
{
    tNodePtr r = root;

    if (!r)
        m_info.branchChanged = false;
    else if (r->left) {
        ExtractMin(r->left);
        if (branchChanged)
            AvlBalanceLShrink(r, branchChanged);
    }
    else {
        tNodePtr d = r;
        m_info.data = r->data;
        r = nullptr;
        delete d;
        branchChanged = true;
    }
    root = r;
}

//-----------------------------------------------------------------------------

public:
bool ExtractMin(DATA_T& data)
{
    if (!m_info.root)
        return false;
    m_info.branchChanged = false;
    ExtractMin(m_info.root, m_info.branchChanged);
    data = m_info.data;
    return true;
}

//-----------------------------------------------------------------------------

private:
void ExtractMax(tNodePtr& root, bool& branchChanged)
{
    tNodePtr r = root;

    if (!r)
        m_info.branchChanged = false;
    else if (r->right) {
        ExtractMax(r->right);
        if (branchChanged)
            AvlBalanceRShrink(r, branchChanged);
    }
    else {
        tNodePtr d = r;
        m_info.data = r->data;
        r = nullptr;
        delete d;
        branchChanged = true;
    }
    root = r;
}

//-----------------------------------------------------------------------------

public:
bool ExtractMax(DATA_T& data)
{
    if (!m_info.root)
        return false;
    m_info.branchChanged = false;
    ExtractMax(m_info.root, m_info.branchChanged);
    data = m_info.data;
    return true;
}

//-----------------------------------------------------------------------------

public:
bool Update(KEY_T oldKey, KEY_T newKey)
{
    if (!Remove(oldKey))
        return false;
    if (!Insert(newKey, m_info.data))
        return false;
    return true;
}

//-----------------------------------------------------------------------------

public:
inline DATA_T& operator[] (KEY_T key)
{
    DATA_T* p = Find(key);
    return p ? *p : throw std::invalid_argument("not found");
}

//-----------------------------------------------------------------------------

public:
    inline CAvlTree& operator= (std::initializer_list<std::pair<KEY_T, DATA_T>> data)
    {
        for (auto d : data)
            Insert(d.first, d.second);
        return *this;
    }

    //-----------------------------------------------------------------------------

};

