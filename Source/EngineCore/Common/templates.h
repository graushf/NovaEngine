#pragma once

//========================================================================
// Templates.h - Some useful templates
//========================================================================


// --------------------------------------------------------------------------------------------------
// In order to dereference a weak_ptr, you have to cast it to a shared_ptr first. You still have to 
// check to see if the pointer is dead dead by calling expired() on the weak_ptr, so why not just allow
// the weak_ptr ot be derefereanceable? It doesn't by anything to force this extra step because
// you can still cast a dead weak_ptr to a shared_ptr and crash. Nice. Anyway, this function takes 
// some of that headache away.
// --------------------------------------------------------------------------------------------------
template <class Type>
shared_ptr<Type> MakeStrongPtr(weak_ptr<Type> pWeakPtr)
{
	if (!pWeakPtr.expired())
		return std::shared_ptr<Type>(pWeakPtr);
	else
		return std::shared_ptr<Type>();
}

template <class BaseType, class SubType>
BaseType* GenericObjectCreationFunction(void) { return new SubType; }

template <class BaseClass, class IdType>
class GenericObjectFactory
{
	typedef BaseClass* (*ObjectCreationFunction)(void);
	std::map<IdType, ObjectCreationFunction> m_creationFunctions;

public:
	template <class SubClass>
	bool Register(IdType id)
	{
		auto findIt = m_creationFunctions.find(id);
		if (findIt == m_creationFunctions.end())
		{
			m_creationFunctions[id] = &GenericObjectCreationFunction<BaseClass, SubClass>; // insert() giving compiler errors
			return true;
		}
		return false;
	}

	BaseClass* Create(IdType id)
	{
		auto findIt = m_creationFunctions.find(id);
		if (findIt != m_creationFunctions.end())
		{
			ObjectCreationFunction pFunc = findIt->second;
			return pFunc();
		}

		return NULL;
	}
};