#pragma once

// Create automatic getter and setter.
#define LAVA_SYNTHESIZE(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const { return varName; }\
public: virtual void set##funName(varType var){ varName = var; }

// Create automatic setter.
#define LAVA_SYNTHESIZE_WRITEONLY(varType, varName, funName)\
protected: varType varName;\
public: virtual void set##funName(varType var){ varName = var; }

// Create automatic getter and setter with pass by reference.
#define LAVA_SYNTHESIZE_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void) const { return varName; }\
public: virtual void set##funName(const varType& var){ varName = var; }

// Create automatic setter with pass by reference.
#define LAVA_SYNTHESIZE_WRITEONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual void set##funName(const varType& var){ varName = var; }

// Create automatic getter for readonly.
#define LAVA_SYNTHESIZE_READONLY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const { return varName; }\

// Create only getter header method.
#define LAVA_PROPERTY_READONLY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const;

// Create only getter header method with pass by reference.
#define LAVA_PROPERTY_READONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void) const;

// Create only getter and setter header.
#define LAVA_PROPERTY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const;\
public: virtual void set##funName(varType var);

// Create only getter and setter header with pass by reference.
#define LAVA_PROPERTY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void) const;\
public: virtual void set##funName(const varType& var);

#define LAVA_TO_STR(A) #A
