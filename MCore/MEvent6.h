/*
*	Event6 
*		6 parameter event & listener
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MMemory.h"

namespace Rad {

	template<class P1, class P2, class P3, class P4, class P5, class P6>
	class _tListener6 : public _tListener
	{
	public:
		_tListener6(void * thiz) : _tListener(thiz) {}
		virtual ~_tListener6() {}

		virtual void OnCall(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) = 0;
	};

	template<class P1, class P2, class P3, class P4, class P5, class P6>
	class tEvent6 : public _tEvent<_tListener6<P1, P2, P3, P4, P5, P6> >
	{
	public:
		void operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
		{
			_tListener * node = mHead;
			_tListener * next = NULL;

			while (node != NULL)
			{
				next = node->_Next;

				static_cast<_tListener2<P1, P2, P3, P4, P5, P6> *>(node)(p1, p2, p3, p4, p5, p6);

				node = next;
			}
		}
	};

	template <class T, class P1, class P2, class P3, class P4, class P5, class P6>
	class cListener6 : public _tListener6<P1, P2, P3, P4, P5, P6>
	{
		typedef void (T::*Function)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

		Function _Fn;

	public:
		cListener6() : _tListener6(NULL), _Fn(NULL) {}
		cListener6(T * _listener, Function _func)  : _tListener6(_listener), _Fn(_func) {}
		virtual ~cListener6() {}

		virtual void OnCall(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
		{
			d_assert (_This != NULL && _Fn != NULL);

			T * thiz = (T *)_This;

			(thiz->*_Fn)(p1, p2, p3, p4, p5, p6);
		}

		cListener6 * operator()(T * _listener, Function _func)
		{
			_This = _listener;
			_Fn = _func;

			return this;
		}
	};

	template<class P1, class P2, class P3, class P4, class P5, class P6>
	class ncListener6 : public _tListener6<P1, P2, P3, P4, P5, P6>
	{
		typedef void (*Function)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

		Function _Fn;

	public:
		ncListener6(Function _func) : _tListener6(NULL), _Fn(_func) {}
		virtual ~ncListener6() {}

		virtual void OnCall(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
		{
			d_assert (_Fn != NULL);

			_Fn(p1, p2, p3, p4, p5, p6);
		}
	};

}
