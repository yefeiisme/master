#ifndef KMemStack_H
#define KMemStack_H

#define MAX_CHUNK	10

class KMemStack
{
private:
	PBYTE		m_pStack[MAX_CHUNK];
	int			m_nStackTop;
	int 		m_nStackEnd;
	int			m_nChunkTop;
	int			m_nChunkSize;
public:
	KMemStack();
	~KMemStack();
	void		Init(int nChunkSize = 65536);
	PVOID		Push(int nSize);
	void		Free(PVOID pMem);
	BOOL		AllocNewChunk();
	void		FreeAllChunks();
	int			GetChunkSize();
	int			GetStackSize();
};

#endif
