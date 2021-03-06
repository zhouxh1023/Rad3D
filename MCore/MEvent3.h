/*
*	Event3 
*		3 parameter event & listener
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MEvent.h"

namespace Rad {

	template<class P1, class P2, class P3>
	class _tListener3 : public _tListener
	{
	public:
		virtual ~_tListener3() {}

		virtual void OnCall(P1 p1, P2 p2, P3 p3) = 0;
	};

	template<class P1, class P2, class P3>
	class tEvent3 : public _tEvent
	{
		typedef _tListener3<P1, P2, P3> listener_t;

	public:
		void operator +=(listener_t * _listener)
		{
			Attach(_listener);
		}

		void operator -=(listener_t * _listener)
		{
			Detach(_listener);
		}

		void operator ()(P1 p1, P2 p2, P3 p3)
		{
			_tListener * node = mHead;
			_tListener * next = NULL;

			while (node != NULL)
			{
				next = node->_Next;

				static_cast<listener_t *>(node)->OnCall(p1, p2, p3);

				node = next;
			}
		}
	};

	template <class T, class P1, class P2, class P3>
	class cListener3 : public _tListener3<P1, P2, P3>
	{
		typedef void (T::*Function)(P1 p1, P2 p2, P3 p3);

		T * _This;
		Function _Fn;

	public:
		cListener3() : _This(NULL), _Fn(NULL) {}
		cListener3(T * _listener, Function _func)  : _This(_listener), _Fn(_func) {}
		virtual ~cListener3() {}

		virtual void OnCall(P1 p1, P2 p2, P3 p3)
		{
			d_assert (_This != NULL && _Fn != NULL);

			(_This->*_Fn)(p1, p2, p3);
		}

		cListener3 * operator()(T * _listener, Function _func)
		{
			_This = _listener;
			_Fn = _func;

			return this;
		}
	};

	template<class P1, class P2, class P3>
	class ncListener3 : public _tListener3<P1, P2, P3>
	{
		typedef void (*Function)(P1 p1, P2 p2, P3 p3);

		Function _Fn;

	public:
		ncListener3(Function _func) : _Fn(_func) {}
		virtual ~ncListener3() {}

		virtual void OnCall(P1 p1, P2 p2, P3 p3)
		{
			d_assert (_Fn != NULL);

			_Fn(p1, p2, p3);
		}
	};

}
