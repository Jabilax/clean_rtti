 //------------------------------------------------------------------------------  
 // <auto-generated>                                                               
 //    This code was generated from a parser.                                      
 //                                                                                
 //    Manual changes to this file will be overwritten if the code is regenerated. 
 // </auto-generated>                                                              
 //------------------------------------------------------------------------------  
#pragma once
#include "../reflection_forward.h"

struct Person;
template<class T> auto reflect_name() -> Specialize<Person, T, std::string> { return "Person"; }
template<class T, int index> auto reflect_variable_name() -> SpecializeIndex<Person, T, 0, index, std::string> { return "name"; }
template<class T, int index> auto reflect_variable_name() -> SpecializeIndex<Person, T, 1, index, std::string> { return "age"; }

struct Human;
template<class T> auto reflect_name() -> Specialize<Human, T, std::string> { return "Human"; }
template<class T, int index> auto reflect_variable_name() -> SpecializeIndex<Human, T, 0, index, std::string> { return "some_variable"; }
template<class T, int index> auto reflect_variable_name() -> SpecializeIndex<Human, T, 1, index, std::string> { return "slider"; }

struct Alien;
template<class T> auto reflect_name() -> Specialize<Alien, T, std::string> { return "Alien"; }

