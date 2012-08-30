/*
	\file		protector.h
	
	\remarks	The Protector class is a Value* class that contains
				a list of ValueWrapper*'s that have been created to protect
				them during MAXScript garbage collection

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#ifndef		__PROTECTOR_H__
#define		__PROTECTOR_H__

applyable_class( Protector );

struct ValueWrapper;

class Protector : public Value {
public:
	Protector();
	~Protector();

	static Protector * sInstance;

	classof_methods( Protector, Value );
	void		collect() { delete this; }
	void		gc_trace();
	Value*		get_property( Value** arg_list, int count );

	static void	protect( ValueWrapper * );
	static void unprotect( ValueWrapper * );

protected:
	int mCount;
	ValueWrapper * mFirst;
};

#endif		__PROTECTOR_H__