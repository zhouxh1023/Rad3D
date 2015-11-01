/*
*	Event0
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MEvent.h"

namespace Rad {

	class _tListener0 : public _tListener
	{
	public:
		_tListener0(void * thiz) : _tListener(thiz) {}
		virtual ~_tListener0() {}

		virtual void OnCall() = 0;
	};

	class tEvent0 : public _tEvent<_tListener0>
	{
	public:
		void operator ()()
		{
			_tListener * node = mHead;
			_tListener * next = NULL;

			while (node != NULL)
			{
				next = node->_Next;

				static_cast<_tListener0 *>(node)->OnCall();

				node = next;
			}
		}
	};

	template <class T>
	class cListener0 : public _tListener0
	{
		typedef void (T::*Function)();

		Function _Fn;

	public:
		cListener0() : _tListener0(NULL), _Fn(NULL) {}
		cListener0(T * _listener, Function _func)  : _tListener0(_listener), _Fn(_func){}
		virtual ~cListener0() {}

		virtual void OnCall()
		{
			d_assert (_This != NULL && _Fn != NULL);

			T * thiz = (T *)_This;

			(thiz->*_Fn)();
		}

		cListener0 * operator()(T * _listener, Function _func)
		{
			_This = _listener;
			_Fn = _func;

			return this;
		}
	};

	class ncListener0 : public _tListener0
	{
		typedef void (*Function)();

		Function _Fn;

	public:
		ncListener0(Function _func) : _tListener0(NULL), _Fn(_func) {}
		virtual ~ncListener0() {}

		virtual void OnCall()
		{
			d_assert (_Fn != NULL);

			_Fn();
		}
	};
}
