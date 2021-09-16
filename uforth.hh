#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <cassert>

using namespace std;

// http://www.stroustrup.com/bs_faq2.html#constraints
template<class T, class B>
struct Derived_from {
  static void constraints(T* p __attribute__((unused))) {
    B* pb __attribute__((unused)) = p;
  }
  Derived_from() { void(*p)(T*) __attribute__((unused)) = constraints; }
};


class ufObject;

typedef vector< shared_ptr< ufObject > > instructions;
typedef deque< shared_ptr< ufObject > > workStack;
typedef unordered_map< string, shared_ptr< ufObject > > dictionary;

class environment {
public:
  environment( environment* parent ) : outer( parent ) {}

  shared_ptr< ufObject >& operator[]( const string& k ) {
    if( core.count( k ) == 0 && outer && outer->count( k ) > 0 ) {
      return outer->operator[]( k );
    }
    return core[ k ];
  }

  size_t count( const string& k ) {
    if( core.count( k ) == 0 && outer ) {
      return outer->count( k );
    }
    return core.count( k );
  }

  bool insert( pair< string, shared_ptr< ufObject > > val ) {
    auto res = core.insert( val );
    return res.second;
  }

protected:
  unordered_map< string, shared_ptr< ufObject > > core;
  environment* outer;
};

template< typename T >
class ufPrimOp {
public:
  typedef T base_type;
  virtual ~ufPrimOp() {}
  virtual T eval( int left, int right ) = 0;
  virtual string str() = 0;
};

class ufInteger;

class ufAddOp : public ufPrimOp< int > {
public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left + right;
  }

  string str() {
    return "+";
  }
};


class ufSubOp : public ufPrimOp< int > {
public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left - right;
  }

  string str() {
    return "-";
  }
};


class ufMulOp : public ufPrimOp< int > {
public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left * right;
  }

  string str() {
    return "*";
  }
};


class ufDivOp : public ufPrimOp< int > {
public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left / right;
  }

  string str() {
    return "/";
  }
};


class ufExpOp : public ufPrimOp< int > {
public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return static_cast< int >( pow( left, right ) );
  }

  string str() {
    return "^";
  }
};

class ufModOp : public ufPrimOp< int > {
public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left % right;
  }

  string str() {
    return "%";
  }
};

class ufBoolean;

class ufLtOp : public ufPrimOp< bool > {
public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left < right;
  }

  string str() {
    return "<";
  }
};

class ufLeOp : public ufPrimOp< bool > {
public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left <= right;
  }

  string str() {
    return "<=";
  }
};

class ufGtOp : public ufPrimOp< bool > {
public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left > right;
  }

  string str() {
    return ">";
  }
};

class ufGeOp : public ufPrimOp< bool > {
public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left >= right;
  }

  string str() {
    return ">=";
  }
};

class ufEqOp : public ufPrimOp< bool > {
public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left == right;
  }

  string str() {
    return "==";
  }
};

class ufNeOp : public ufPrimOp< bool > {
public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left != right;
  }

  string str() {
    return "!=";
  }
};

class ufObject {
public:
  virtual ~ufObject() {}
  virtual void eval( workStack&, environment& ) = 0;
  virtual string str() = 0;
};

class ufSymbol : public ufObject {
public:
  ufSymbol( string s ) : value( s ) {}
  ~ufSymbol() {}

  void eval( workStack& theStack, environment& theEnv ) {
    if( theEnv.count( value ) ) {
      auto b = theEnv[ value ];
      b->eval( theStack, theEnv );
    }
    else {
      theStack.push_front( make_shared< ufSymbol >( value ) );
    }
  }

  string str() {
    return value;
  }

protected:
  string value;
};

class ufInteger : public ufObject {
public:
  ufInteger( int number ) : value( number ) {}

  void eval( workStack& theStack, environment& ) {
    theStack.push_front( make_shared< ufInteger >( value ) );
  }

  string str() {
    return to_string( value );
  }

  int val() {
    return value;
  }

protected:
  int value;
};

class ufNegOp : public ufObject {
public:
  void eval( workStack& theStack, environment& ) {
    auto data = theStack.front();
    theStack.pop_front();

    int rData = static_cast< ufInteger* >( data.get() )->val();

    theStack.push_front( make_shared< ufInteger >( -1 * rData ) );
  }

  string str() {
    return "~";
  }

};

template< typename T >
class ufBinOp : public ufObject, Derived_from< T, ufPrimOp< typename T::base_type > > {
public:

  void eval( workStack& theStack, environment& ) {
    assert( theStack.size() > 1 );
    auto right = theStack.front();
    theStack.pop_front();
    auto left = theStack.front();
    theStack.pop_front();

    int rInt = static_cast< ufInteger* >( right.get() )->val();
    int lInt = static_cast< ufInteger* >( left.get() )->val();

    theStack.push_front( make_shared< typename T::value_type >( op.eval( lInt, rInt ) ) );
  }

  string str() {
    return op.str();
  }

protected:
  T op;
};

const string blockBegin = "{";
const string blockEnd = "}";

class ufBeginBlock : public ufObject {
public:
  void eval( workStack& theStack, environment& ) {
    theStack.push_front( make_shared< ufBeginBlock >() );
  }

  string str() {
    return blockBegin;
  }
};

enum modes { evaluate, define };

class ufBlock : public ufObject {
public:

  void eval( workStack& theStack, environment& theEnv ) {
    modes mode = evaluate;
    int blockDepth = 0;
    environment innerEnv( &theEnv );

    for( auto i : insns ) {
      if( i->str() == blockBegin ) {
        blockDepth++;
      }
      else if( i->str() == blockEnd ) {
        blockDepth--;
      }

      mode = blockDepth ? define : evaluate;

      switch( mode ) {
      case evaluate:
        i->eval( theStack, innerEnv );
        break;
      case define:
        theStack.push_front( i );
        break;
      }
    }
  }

  string str() {
    stringstream buffer;

    buffer << blockBegin << " ";

    for( auto i : insns ) {
      buffer << i->str() << " ";
    }

    buffer << blockEnd;
    return buffer.str();
  }

  void addIns( shared_ptr< ufObject >& ins ) {
    insns.insert( insns.begin(), ins );
  }

protected:
  instructions insns;
};

class ufMkBlock : public ufObject {
public:
  void eval( workStack& theStack, environment& ) {
    assert( theStack.size() > 0 );
    auto function = make_shared< ufBlock >();
    int blockDepth = 1;

    while( blockDepth ) {
      if( theStack.front()->str() == blockBegin ) {
        blockDepth--;
      }
      else if( theStack.front()->str() == blockEnd ) {
        blockDepth++;
      }

      if( blockDepth ) {
        function->addIns( theStack.front() );
        theStack.pop_front();
      }
    }
    theStack.pop_front(); // eat the "("
    theStack.push_front( function );
  }

  string str() {
    return blockEnd;
  }
};

class ufDupOp : public ufObject {
public:
  void eval( workStack& theStack, environment& ) {
    assert( theStack.size() > 0 );
    auto top = static_cast< ufInteger* >( theStack.front().get() )->val();
    auto dup = make_shared< ufInteger >( top );
    theStack.push_front( dup );
  }

  string str() {
    return "dup";
  }
};

class ufSwapOp : public ufObject {
public:
  void eval( workStack& theStack, environment& ) {
    assert( theStack.size() > 1 );
    auto first = theStack.front();
    theStack.pop_front();
    auto second = theStack.front();
    theStack.pop_front();

    theStack.push_front( first );
    theStack.push_front( second );
  }

  string str() {
    return "swap";
  }
};

class ufPopOp : public ufObject {
public:
  void eval( workStack& theStack, environment& ) {
    assert( theStack.size() > 0 );
    theStack.pop_front();
  }

  string str() {
    return "_";
  }
};

class ufAssignOp : public ufObject {
public:
  void eval( workStack& theStack, environment& theEnv ) {
    assert( theStack.size() > 1 );
    auto right =  theStack.front();
    theStack.pop_front();

    auto left = theStack.front();
    theStack.pop_front();

    pair< string, shared_ptr< ufObject > >thing ( left->str(), right );
    theEnv.insert( thing );
  }

  string str() {
    return "=";
  }
};

const string strue = "true";
const string sfalse = "false";

class ufBoolean : public ufObject {
public:
  ufBoolean( bool b ) : value( b ) {}

  void eval( workStack& theStack, environment& theEnv ) {
    assert( theStack.size() > 1 );
    auto thenBlock = theStack.front();
    theStack.pop_front();
    auto elseBlock = theStack.front();
    theStack.pop_front();

    if( value ) {
      thenBlock->eval( theStack, theEnv );
    }
    else {
      elseBlock->eval( theStack, theEnv );
    }
  }

  string str() {
    if( value ) {
      return strue;
    }
    else {
      return sfalse;
    }
  }

protected:
  bool value;
};

class ufIfOp : public ufObject {
public:
  void eval( workStack& theStack, environment& theEnv ) {
    assert( theStack.size() > 2 );
    auto elseBlock = theStack.front(); theStack.pop_front();
    auto thenBlock = theStack.front(); theStack.pop_front();
    auto cond = theStack.front(); theStack.pop_front();

    if( cond->str() == strue ) {
      thenBlock->eval( theStack, theEnv );
    }
    else if( cond->str() == sfalse ) {
      elseBlock->eval( theStack, theEnv );
    }
  }

  string str() {
    return "if";
  }
};

class ufBooleanOp : public ufObject {
public:
  ufBooleanOp( bool v ) : value( v ) {}

  void eval( workStack& theStack, environment& ) {
    theStack.push_front( make_shared< ufBoolean >( value ) );
  }

  string str() {
    if( value ) {
      return strue;
    }
    else {
      return sfalse;
    }
  }

protected:
  bool value;
};

class ufLoopOp : public ufObject {
public:
  void eval( workStack& theStack, environment& theEnv ) {
    assert( theStack.size() > 1 );
    auto whileBlock = theStack.front();
    theStack.pop_front();
    auto block = static_cast< ufBlock* >( whileBlock.get() );

    while( theStack.front()->str() == strue ) {
      theStack.pop_front();
      block->eval( theStack, theEnv );
    }
    theStack.pop_front(); // eat the "false"
  }

  string str() {
    return "loop";
  }
};

#endif
