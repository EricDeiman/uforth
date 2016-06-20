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

using namespace std;

class ufObject;

typedef vector< shared_ptr< ufObject > > instructions;
typedef deque< shared_ptr< ufObject > > workStack;
typedef unordered_map< string, shared_ptr< ufObject > > dictionary;

class ufPrimOp {
 public:
  virtual ~ufPrimOp() {}
  virtual int eval( int left, int right ) = 0;
  virtual string str() = 0;
};

class ufAddOp : public ufPrimOp {
 public:
  int eval( int left, int right ) {
    return left + right;
  }

  string str() {
    return "+";
  }
};

class ufSubOp : public ufPrimOp {
 public:
  int eval( int left, int right ) {
    return left - right;
  }

  string str() {
    return "-";
  }
};

class ufMulOp : public ufPrimOp {
 public:
  int eval( int left, int right ) {
    return left * right;
  }

  string str() {
    return "*";
  }
};

class ufDivOp : public ufPrimOp {
 public:
  int eval( int left, int right ) {
    return left / right;
  }

  string str() {
    return "/";
  }
};

class ufExpOp : public ufPrimOp {
 public:
  int eval( int left, int right ) {
    return static_cast< int >( pow( left, right ) );
  }

  string str() {
    return "^";
  }
};

class ufObject {
 public:
  virtual ~ufObject() {}
  virtual void eval( workStack&, dictionary& ) = 0;
  virtual string str() = 0;
};

class ufSymbol : public ufObject {
 public:
  ufSymbol( string s ) : value( s ) {}
  ~ufSymbol() {}

  void eval( workStack& theStack, dictionary& theEnv ) {
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

  void eval( workStack& theStack, dictionary& ) {
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

class ufBinOp : public ufObject {
 public:

 ufBinOp( ufPrimOp* pop ) : op( pop ) {}

  void eval( workStack& theStack, dictionary& ) {
    auto right = theStack.front();
    theStack.pop_front();
    auto left = theStack.front();
    theStack.pop_front();

    int rInt = static_cast< ufInteger* >( right.get() )->val();
    int lInt = static_cast< ufInteger* >( left.get() )->val();

    theStack.push_front( make_shared< ufInteger >( op->eval( lInt, rInt ) ) );
  }

  string str() {
    return op->str();
  }

  ~ufBinOp() {
    delete op;
  }

 protected:
  ufPrimOp* op;
};

const string blockBegin = "{";
const string blockEnd = "}";

class ufBeginBlock : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& ) {
    theStack.push_front( make_shared< ufBeginBlock >() );
  }

  string str() {
    return blockBegin;
  }
};

class ufBlock : public ufObject {
 public:

  void eval( workStack& theStack, dictionary& theEnv ) {
    for( auto i : insns ) {
      i->eval( theStack, theEnv );
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
  void eval( workStack& theStack, dictionary& ) {
    auto function = make_shared< ufBlock >();
    while( theStack.front()->str() != blockBegin )  {
      function->addIns( theStack.front() );
      theStack.pop_front();
    }
    theStack.pop_front(); // eat the "("
    theStack.push_front( shared_ptr< ufObject >( function ) );
  }

  string str() {
    return "}";
  }
};

class ufDupOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& ) {
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
  void eval( workStack& theStack, dictionary& ) {
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
  void eval( workStack& theStack, dictionary& ) {
    theStack.pop_front();
  }

  string str() {
    return "_";
  }
};

class ufAssignOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& theEnv ) {
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

class ufBoolOp : public ufObject {
 public:
  virtual void condEval( workStack&, dictionary& ) = 0;
};

class ufTrueOp : public ufBoolOp {
 public:
  void eval( workStack& theStack, dictionary& ) {
    theStack.push_front( make_shared< ufTrueOp >() );
  }

  string str() {
    return "true";
  }

  void condEval( workStack& theStack, dictionary& theEnv ) {
    auto thenBlock = theStack.front();
    theStack.pop_front();

    // ignore the else block
    theStack.pop_front();

    thenBlock->eval( theStack, theEnv );
  }
};

class ufFalseOp : public ufBoolOp {
 public:
  void eval( workStack& theStack, dictionary& ) {
    theStack.push_front( make_shared< ufFalseOp >() );
  }

  string str() {
    return "false";
  }

  void condEval( workStack& theStack, dictionary& theEnv ) {
    // ignore the then block
    theStack.pop_front();

    auto elseBlock = theStack.front();
    theStack.pop_front();

    elseBlock->eval( theStack, theEnv );
  }
};

class ufIfOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& theEnv ) {
    auto cond = theStack.front();
    theStack.pop_front();
    auto boolOp = static_cast< ufBoolOp* >( cond.get() );
    boolOp->condEval( theStack, theEnv );
  }

  string str() {
    return "if";
  }
};

#endif
