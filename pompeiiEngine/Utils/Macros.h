/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __POMPEII_ENGINE_MACROS__
#define __POMPEII_ENGINE_MACROS__

// Create automatic getter and setter.
#define POMPEII_SYNTHESIZE(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const { return varName; }\
public: virtual void set##funName(varType var){ varName = var; }

// Create automatic setter.
#define POMPEII_SYNTHESIZE_WRITEONLY(varType, varName, funName)\
protected: varType varName;\
public: virtual void set##funName(varType var){ varName = var; }

// Create automatic getter and setter with pass by reference.
#define POMPEII_SYNTHESIZE_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void) const { return varName; }\
public: virtual void set##funName(const varType& var){ varName = var; }

// Create automatic setter with pass by reference.
#define POMPEII_SYNTHESIZE_WRITEONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual void set##funName(const varType& var){ varName = var; }

// Create automatic getter for readonly.
#define POMPEII_SYNTHESIZE_READONLY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const { return varName; }\

// Create only getter header method.
#define POMPEII_PROPERTY_READONLY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const;

// Create only getter header method with pass by reference.
#define POMPEII_PROPERTY_READONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void) const;

// Create only getter and setter header.
#define POMPEII_PROPERTY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void) const;\
public: virtual void set##funName(varType var);

// Create only getter and setter header with pass by reference.
#define POMPEII_PROPERTY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void) const;\
public: virtual void set##funName(const varType& var);

#define POMPEII_TO_STR(A) #A

#endif /* __POMPEII_ENGINE_MACROS__ */