/*
	\file		wrapper.h

	\remarks	This is the main class definition for the wrapper classes.
				* ValueWrapper is a PyObject* class that wraps a Value* class 
				* ObjectWrapper is a Value* class that wraps a PyObject* class
	
	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#ifndef		__WRAPPER_H__
#define		__WRAPPER_H__

#include	<list>

typedef std::list<PyObject*>	CollectionMap;

visible_class( ObjectWrapper );
class ObjectWrapper : public Value {
private:
	PyObject*						mObject;
	static CollectionMap*			collectionMaps;		// used when mapping a collection to a python list

public:
	ObjectWrapper( PyObject* obj = NULL );
	~ObjectWrapper();

	// maxscript value methods
	__declspec( dllexport ) virtual Value* eq_vf(  Value** arg_list, int arg_count );
	__declspec( dllexport ) virtual Value* get_vf( Value** arg_list, int arg_count );
	__declspec( dllexport ) virtual Value* put_vf( Value** arg_list, int arg_count );

	// maxscript value comparisons
	classof_methods( ObjectWrapper, Value );
#define is_objectwrapper(o) ((o)->tag == class_tag(ObjectWrapper))
#define is_maxmodifierarray(o) ((o)->tag == class_tag(MAXModifierArray))

	// objectwrapper instance methods
	Value*			apply( Value** arg_list, int count, CallContext* cc = 0 );
	virtual void	collect();
	Value*			get_property( Value** arg_list, int count );
	Value*			set_property( Value** arg_list, int count );
	PyObject*		object();
	void			sprin1( CharStream* s );
	char*			to_string();

	BOOL			_is_collection()	{ return 1; }
	BOOL			_is_function()		{ return 1; }

	// objectwrapper static methods
	static Value*		intern( PyObject* obj );
	static Value*		collectionMapper( Value** args, int count );
	static bool			init();
	static void			handleMaxscriptError();
	static void			gc_protect( PyObject* obj );
	static bool			is_wrapper( PyObject* obj );
	static void			log(		PyObject* obj );
	static PyObject*	py_intern( Value* val );
};

#endif		__WRAPPER_H__

/*
Value* 
intersect_nodes(Value** arg_list, int count)
{
	// node tab collector for selection node mapping
	// arg0 is node candidate, arg1 is result Array, arg2 is the ray value
	Value* res = intersectRayScene(((MAXNode*)arg_list[0])->node, arg_list[2]);
	if (res != &undefined) 
	{
		one_typed_value_local(Array* result);
		vl.result = new Array (2);
		vl.result->append(arg_list[0]);
		vl.result->append(res);
		((Array*)arg_list[1])->append(vl.result);
	}
	return &ok;
}

Value*
intersectRayScene_cf(Value** arg_list, int count)
{
	check_arg_count(intersectRayScene, 1, count);
	arg_list[0]->to_ray();
	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	Value* args[3] = { NULL, vl.result, arg_list[0] };
	node_map m = { NULL, intersect_nodes, args, 2 };
	Value* all_objects = globals->get(Name::intern(_T("objects")))->eval();
	all_objects->map(m);
	return_value(vl.result);
}
*/