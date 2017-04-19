#ifndef KLinkArray_H
#define	KLinkArray_H

class KLinkNode
{
public:
	int nPrev;
	int	nNext;
public:
	KLinkNode() { nPrev = nNext = 0; }
};

class KLinkArray
{
private:
	KLinkNode		*pNode;
	int				m_nCount;
	int				m_nSize;
public:
	KLinkArray() { pNode = NULL; m_nCount = 0; m_nSize = 0;}
	~KLinkArray();

	int				GetCount()
	{
		return m_nCount;
	}

	KLinkNode		*GetNode( const int nIdx )
	{
		return &pNode[nIdx];
	}

	void			Init(int nSize);
	void			Remove(int nIdx);
	void			Insert(int nIdx);
	
	int				GetNext( const int nIdx ) const 
	{
		if( nIdx < 0 || nIdx >= m_nSize )
			return 0;

		return ( pNode ? pNode[nIdx].nNext : 0 );
	}

	int				GetPrev( const int nIdx ) const 
	{
		if( nIdx < 0 || nIdx >= m_nSize )
			return 0;

		return ( pNode ? pNode[nIdx].nPrev : 0 );
	}
};
#endif
