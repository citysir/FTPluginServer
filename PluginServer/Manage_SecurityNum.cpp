#include "stdafx.h"
#include "Manage_SecurityNum.h"

Manage_SecurityNum::Manage_SecurityNum(void)
{
}

Manage_SecurityNum::~Manage_SecurityNum(void)
{
}

void Manage_SecurityNum::Clear()
{
	//m_safe.EnterCriticalSection();//



	//m_safe.LeaveCriticalSection();//
}

void Manage_SecurityNum::InsertCookieSocket(const UINT32 &nCookie, const SOCKET &sock)
{
	m_mapCookieSocket.insert(std::make_pair(nCookie, sock));
}

void Manage_SecurityNum::DeleteCookieSocket(const UINT32 &nCookie)
{
	std::map<UINT32, SOCKET>::iterator iter_find = m_mapCookieSocket.begin();
	for (; iter_find != m_mapCookieSocket.end(); iter_find++)
	{
		if (iter_find->first == nCookie)
		{
			iter_find = m_mapCookieSocket.erase(iter_find);
			break;
		}
	}
}

bool Manage_SecurityNum::IsSafeSocket(const SOCKET &sock)
{
	std::vector<SOCKET>::iterator iter_find = m_vtSocket.begin();
	for (; iter_find != m_vtSocket.end(); iter_find++)
	{
		if (*iter_find == sock)
		{
			return true;
		}
	}
	return false;
}

void Manage_SecurityNum::AddSafeSocket(const UINT32 &nCookie)
{
	std::map<UINT32, SOCKET>::iterator iter_find = m_mapCookieSocket.begin();
	for (; iter_find != m_mapCookieSocket.end(); iter_find++)
	{
		if (iter_find->first == nCookie)
		{
			SOCKET sock = iter_find->second;
			if (std::find(m_vtSocket.begin(), m_vtSocket.end(), sock) == m_vtSocket.end())
			{
				m_vtSocket.push_back(iter_find->second);
			}
		}
	}
}

bool Manage_SecurityNum::GetSocketByCookie(UINT32 nCookie, SOCKET &sock)
{
	if (m_mapCookieSocket.count(nCookie) != 0)
	{
		sock = m_mapCookieSocket[nCookie];
		return true;
	}
	return false;
}
