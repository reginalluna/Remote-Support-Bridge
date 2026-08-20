#include "StdAfx.h"
#include "Buffer.h"

#include <limits.h>

#define U_PAGE_ALIGNMENT 3

CBuffer::CBuffer(void)
{
	m_ulMaxLength = 0;
	m_Ptr = m_Base = NULL;
	InitializeCriticalSection(&m_cs);
}

CBuffer::~CBuffer(void)
{
	if (m_Base)
	{
		VirtualFree(m_Base, 0, MEM_RELEASE);
		m_Base = NULL;
	}

	DeleteCriticalSection(&m_cs);
	m_Base = m_Ptr = NULL;
	m_ulMaxLength = 0;
}

ULONG CBuffer::RemoveComletedBuffer(ULONG ulLength)
{
	if (ulLength > GetBufferMaxLength())
		return 0;

	if (ulLength > GetBufferLength())
		ulLength = GetBufferLength();

	if (ulLength)
	{
		MoveMemory(m_Base, m_Base + ulLength, GetBufferMaxLength() - ulLength);
		m_Ptr -= ulLength;
	}

	DeAllocateBuffer(GetBufferLength());
	return ulLength;
}

ULONG CBuffer::ReadBuffer(PBYTE Buffer, ULONG ulLength)
{
	if (Buffer == NULL && ulLength != 0)
		return 0;

	EnterCriticalSection(&m_cs);

	if (ulLength > GetBufferMaxLength())
	{
		LeaveCriticalSection(&m_cs);
		return 0;
	}
	if (ulLength > GetBufferLength())
		ulLength = GetBufferLength();

	if (ulLength)
	{
		CopyMemory(Buffer, m_Base, ulLength);
		MoveMemory(m_Base, m_Base + ulLength, GetBufferMaxLength() - ulLength);
		m_Ptr -= ulLength;
	}

	DeAllocateBuffer(GetBufferLength());
	LeaveCriticalSection(&m_cs);
	return ulLength;
}

ULONG CBuffer::DeAllocateBuffer(ULONG ulLength)
{
	if (ulLength < GetBufferLength() || ulLength > ULONG_MAX - (U_PAGE_ALIGNMENT - 1))
		return 0;

	const ULONG ulNewMaxLength = ((ulLength + U_PAGE_ALIGNMENT - 1) / U_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;
	if (GetBufferMaxLength() <= ulNewMaxLength)
		return 0;

	PBYTE NewBase = (PBYTE)VirtualAlloc(NULL, ulNewMaxLength, MEM_COMMIT, PAGE_READWRITE);
	if (NewBase == NULL)
		return 0;

	const ULONG ulv1 = GetBufferLength();
	if (m_Base != NULL && ulv1 != 0)
		CopyMemory(NewBase, m_Base, ulv1);

	if (m_Base != NULL)
		VirtualFree(m_Base, 0, MEM_RELEASE);

	m_Base = NewBase;
	m_Ptr = m_Base + ulv1;
	m_ulMaxLength = ulNewMaxLength;
	return m_ulMaxLength;
}

BOOL CBuffer::WriteBuffer(PBYTE Buffer, ULONG ulLength)
{
	if (Buffer == NULL && ulLength != 0)
		return FALSE;

	EnterCriticalSection(&m_cs);

	const ULONG currentLength = GetBufferLength();
	if (ulLength > ULONG_MAX - currentLength || ReAllocateBuffer(ulLength + currentLength) == (ULONG)-1)
	{
		LeaveCriticalSection(&m_cs);
		return FALSE;
	}

	if (ulLength != 0)
	{
		CopyMemory(m_Ptr, Buffer, ulLength);
		m_Ptr += ulLength;
	}

	LeaveCriticalSection(&m_cs);
	return TRUE;
}

ULONG CBuffer::ReAllocateBuffer(ULONG ulLength)
{
	if (ulLength < GetBufferMaxLength())
		return 0;

	if (ulLength > ULONG_MAX - (U_PAGE_ALIGNMENT - 1))
		return (ULONG)-1;

	const ULONG ulNewMaxLength = ((ulLength + U_PAGE_ALIGNMENT - 1) / U_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;
	PBYTE NewBase = (PBYTE)VirtualAlloc(NULL, ulNewMaxLength, MEM_COMMIT, PAGE_READWRITE);
	if (NewBase == NULL)
		return (ULONG)-1;

	const ULONG ulv1 = GetBufferLength();
	if (m_Base != NULL && ulv1 != 0)
		CopyMemory(NewBase, m_Base, ulv1);

	if (m_Base)
		VirtualFree(m_Base, 0, MEM_RELEASE);

	m_Base = NewBase;
	m_Ptr = m_Base + ulv1;
	m_ulMaxLength = ulNewMaxLength;
	return m_ulMaxLength;
}

VOID CBuffer::ClearBuffer()
{
	EnterCriticalSection(&m_cs);
	m_Ptr = m_Base;
	DeAllocateBuffer(1024);
	LeaveCriticalSection(&m_cs);
}

ULONG CBuffer::GetBufferLength()
{
	if (m_Base == NULL || m_Ptr == NULL || m_Ptr < m_Base)
		return 0;

	const size_t length = static_cast<size_t>(m_Ptr - m_Base);
	if (length > ULONG_MAX)
		return 0;

	return static_cast<ULONG>(length);
}

ULONG CBuffer::GetBufferMaxLength()
{
	return m_ulMaxLength;
}

PBYTE CBuffer::GetBuffer(ULONG ulPos)
{
	if (m_Base == NULL || ulPos >= GetBufferLength())
		return NULL;

	return m_Base + ulPos;
}
