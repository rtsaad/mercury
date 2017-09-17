#ifndef _TBB_H_
#define _TBB_H_

#ifdef __cplusplus
 
//#include <cstring>
#include <strings.h>

#include <tbb/concurrent_unordered_map.h>
 

extern "C" {
#endif
  
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
struct data
{
  
  char*        content;
  unsigned int size;
  
  data()
    : content(NULL)
    , size(0)
  {
  }
  
  data( char* c, unsigned int s )
    : content(c)
    , size(s)
  {
  }
  
  bool
  operator==( const data& rhs )
  const
  {
    if( this->size != rhs.size )
    {
      return false;
    }
    return (memcmp( this->content, rhs.content, this->size) == 0);
  }
  
};

struct dummy
{
    /*char* element;
    dummy(char* c, int size){
        element = (char*) malloc(size*sizeof(char));
        memcpy(element, c, size);
    }*/
};

struct tbb_hash_table
{
  
  struct hasher
  {
    size_t (*hash_function)( char*, unsigned int );
    
    hasher( size_t (*hf)( char*, unsigned int ) )
      : hash_function(hf)
    {
    }
    
    std::size_t
    operator()( const data& x )
    const
    {
      return (*hash_function)( x.content, x.size );
    }
  };

  typedef tbb::concurrent_unordered_map< data, dummy, hasher >
          container_type;
  
  container_type container;
  
  tbb_hash_table( unsigned int nb_buckets, size_t (*hash_function)( char*, unsigned int ) )
    : container( nb_buckets, hasher(hash_function) )
  {
  }
  
};

#else // C interface
typedef struct tbb_hash_table tbb_hash_table;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

tbb_hash_table*
new_tbb_hash_table( unsigned int, size_t (*)( char*, unsigned int) );

////////////////////////////////////////////////////////////////////////////////////////////////////

void
delete_tbb_hash_table( tbb_hash_table* );

////////////////////////////////////////////////////////////////////////////////////////////////////

int
tbb_hash_table_insert( tbb_hash_table*, char*, unsigned int );

////////////////////////////////////////////////////////////////////////////////////////////////////

int
tbb_hash_table_lookup( tbb_hash_table*, char*, unsigned int );

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // _TBB_H_
