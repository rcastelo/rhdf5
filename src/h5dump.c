#include "H5G.h"
#include <stdio.h>
#include "printdatatype.h"

/*################################*/
/* constants */
/*################################*/

/*################################*/
/* functions */
/*################################*/

typedef struct opLinfoTreeElement {
  long idx;
  char *name;
  char *group;
  char *datatype;
  char *class;
  char *spacetype;
  int rank;
  char *dim;
  H5L_info_t info;
  H5O_info_t object_info;
  struct opLinfoTreeElement *child;
  struct opLinfoTreeElement *next;
} opLinfoTreeElement;

typedef struct {
  long n;
  long depth;
  char *group;
  long maxdepth;
  int objecttype;
  int showdatasetinfo;
  H5_index_t index_type;
  H5_iter_order_t order;
  opLinfoTreeElement *first;
  opLinfoTreeElement *last;
  int insertAsChild;
} opLinfoTree;

herr_t opAddToLinfoTree( hid_t g_id, const char *name, const H5L_info_t *info, void *op_data) {
  opLinfoTree *data = op_data;

  opLinfoTreeElement *newElement = malloc(sizeof(struct opLinfoTreeElement) );
  newElement->idx = data->n;
  /* printf("sizeof = %ld cset=%ld group=>%s< name=>%s<\n", strlen(name), info->cset, data->group, name); */
  newElement->name = malloc((strlen(name)+1)*sizeof(char));
  strcpy(newElement->name, name);
  newElement->group = malloc((strlen(data->group)+1)*sizeof(char));
  strcpy(newElement->group, data->group);
  newElement->info = (*info);

  hid_t oid = H5Oopen( g_id, name, H5P_DEFAULT );
  herr_t herr = H5Oget_info( oid, &(newElement->object_info) );
  if ((data->showdatasetinfo > 0) & (newElement->object_info.type == H5O_TYPE_DATASET)) {
    hid_t did = H5Dopen( g_id, name, H5P_DEFAULT );
    hid_t type = H5Dget_type(did);
    newElement->datatype = getDatatypeName(type);
    newElement->class = getDatatypeClass(type);
    /* H5Tclose(type); */
    hid_t sid = H5Dget_space( did );
    hsize_t   size[H5S_MAX_RANK];
    hsize_t   maxsize[H5S_MAX_RANK];
    newElement->rank = H5Sget_simple_extent_dims(sid, size, maxsize);

    H5S_class_t space_type = H5Sget_simple_extent_type(sid);
    switch(space_type) {
    case H5S_SCALAR:   newElement->spacetype = "SCALAR"; break;
    case H5S_SIMPLE:   newElement->spacetype = "SIMPLE"; break;
    case H5S_NULL:     newElement->spacetype = "NULL"; break;
    case H5S_NO_CLASS: newElement->spacetype = "NO_CLASS"; break;
    default:           newElement->spacetype = "unknown dataspace"; break;
    } /* end switch */
    newElement->dim = "";
    switch(space_type) {
    case H5S_SCALAR: newElement->dim = "( 0 )"; break;
    case H5S_SIMPLE: {
      char* tmp = malloc(100*newElement->rank*sizeof(char));
      sprintf(tmp, "( %lu", size[newElement->rank-1]);
      for(int i = newElement->rank-2; i >= 0 ; i--) {
      	sprintf(tmp, "%s, %lu", tmp, size[i]);
      }
      sprintf(tmp, "%s ) / ", tmp);
      if(maxsize[0] == H5S_UNLIMITED) {
	sprintf(tmp, "%sUNLIMITED", tmp);
      } else {
	sprintf(tmp, "%s( %lu", tmp, maxsize[newElement->rank-1]);
	for(int i = newElement->rank-2; i >= 0 ; i--) {
	  sprintf(tmp, "%s, %lu", tmp, maxsize[i]);
	}
	sprintf(tmp, "%s )", tmp);
      }
      newElement->dim = malloc((strlen(tmp)+1)*sizeof(char));
      strcpy(newElement->dim, tmp);
      free(tmp);
    } break;
    case H5S_NULL: newElement->dim = ""; break;
    case H5S_NO_CLASS:
    default: newElement->dim = "unknown dataspace"; break;
    } /* end switch */
    H5Sclose(sid);

    /* printf("type=%ld\n",H5T_STD_I32LE); */
    /* printf("type=%ld\n",H5T_IEEE_F32LE); */
    /* const char *typename = getDatatypeName(type, 1); */
    /* printf("type=%ld\n",hid); */
    /* char *typename; */
    /* typename = malloc(1001*sizeof(char)); */
    /* ssize_t s = H5Iget_name( hid, typename, 1000 ); */
    /* printf("size=%ld\n",s); */
    /* printf("name=%s\n\n",typename); */
    H5Dclose(did);
  } else {
    newElement->datatype = "";
    newElement->class = "";
    newElement->rank = 0;
    newElement->spacetype = "";
    newElement->dim = "";
  }

  newElement->child = NULL;
  newElement->next = NULL;

  if ((data->objecttype < 0) | (newElement->object_info.type == H5O_TYPE_GROUP) | (newElement->object_info.type == data->objecttype)) {
    data->n = data->n + 1;
    if (data->first == NULL) {
      data->first = newElement;
    } else {
      if (data->insertAsChild) {
	data->last->child = newElement;
	data->insertAsChild = 0;
      } else {
	data->last->next = newElement;
      }
    }
    data->last = newElement;
    /* printf("gid=%d name=%s n=%ld\n",g_id, name, data->n); */
  }

  if ((herr >= 0) & (newElement->object_info.type == H5O_TYPE_GROUP)) {
    if ((data->maxdepth < 0) | (data->depth < data->maxdepth)) {
      hsize_t idx=0;
      char* group = data->group;
      data->group = malloc((strlen(name)+strlen(group)+2)*sizeof(char));
      strcpy(data->group, group);
      if (data->depth > 1) {
	strcat(data->group, "/");
      }
      strcat(data->group, name);
      data->insertAsChild = 1;
      opLinfoTreeElement *last = data->last;
      data->depth = data->depth + 1;
      herr = H5Literate( oid, data->index_type, data->order, &idx, &opAddToLinfoTree, op_data );
      data->depth = data->depth - 1;
      data->insertAsChild = 0;
      data->last = last;
      free(data->group);
      data->group = group;
    }
  }
  H5Oclose(oid);

  if (!((data->objecttype < 0) | (newElement->object_info.type == H5O_TYPE_GROUP) | (newElement->object_info.type == data->objecttype))) {
    free(newElement);
  }
  return(herr);
}

SEXP
getTree(opLinfoTreeElement* elstart, opLinfoTree* data, hid_t loc_id, int depth) {
  int n=0;
  opLinfoTreeElement *el = elstart;
  while (el != NULL) {
    n = n + 1;
    el = el->next;
  }
  SEXP Rval;
  PROTECT(Rval= allocVector(VECSXP, n));
  SEXP names = PROTECT(allocVector(STRSXP, n));

  n=0;
  el = elstart;
  while (el != NULL) {

    /* printTabs(depth); */
    /* printf("%d\t%10s\t%20s\t%50s\n",el->object_info.type, el->name, el->datatype, el->group); */

    SET_STRING_ELT(names, n, mkChar(el->name));

    if (el->child != NULL) {
      SEXP childtree = getTree(el->child, data, loc_id, depth+1);
      SET_VECTOR_ELT(Rval,n,childtree);
    } else {
      if (el->object_info.type == H5O_TYPE_GROUP) {
	SET_VECTOR_ELT(Rval,n,R_NilValue);
      } else {
	SEXP info = PROTECT(allocVector(VECSXP, 20));
	SET_VECTOR_ELT(info,0,mkString("/"));  
	SET_VECTOR_ELT(info,1,mkString(el->name));  
	SET_VECTOR_ELT(info,2,ScalarInteger(el->info.type));  
	SET_VECTOR_ELT(info,3,ScalarLogical(el->info.corder_valid)); 
	SET_VECTOR_ELT(info,4,ScalarInteger(el->info.corder)); 
	SET_VECTOR_ELT(info,5,ScalarInteger(el->info.cset));
	SET_VECTOR_ELT(info,6,ScalarInteger(el->object_info.fileno));
	SET_VECTOR_ELT(info,7,ScalarInteger(el->object_info.addr));
	SET_VECTOR_ELT(info,8,ScalarInteger(el->object_info.type));
	SET_VECTOR_ELT(info,9,ScalarInteger(el->object_info.rc)); 
	SET_VECTOR_ELT(info,10,ScalarReal(el->object_info.atime)); 
	SET_VECTOR_ELT(info,11,ScalarReal(el->object_info.mtime));
	SET_VECTOR_ELT(info,12,ScalarReal(el->object_info.ctime));
	SET_VECTOR_ELT(info,13,ScalarReal(el->object_info.btime));
	SET_VECTOR_ELT(info,14,ScalarInteger(el->object_info.num_attrs));
	SET_VECTOR_ELT(info,15,mkString(el->class));  
	SET_VECTOR_ELT(info,16,mkString(el->datatype)); 
	SET_VECTOR_ELT(info,17,mkString(el->spacetype)); 
	SET_VECTOR_ELT(info,18,ScalarInteger(el->rank));
	SET_VECTOR_ELT(info,19,mkString(el->dim));
	
	SEXP infonames = PROTECT(allocVector(STRSXP, 20));
	SET_STRING_ELT(infonames, 0, mkChar("group"));
	SET_STRING_ELT(infonames, 1, mkChar("name"));
	SET_STRING_ELT(infonames, 2, mkChar("ltype"));
	SET_STRING_ELT(infonames, 3, mkChar("corder_valid"));
	SET_STRING_ELT(infonames, 4, mkChar("corder"));
	SET_STRING_ELT(infonames, 5, mkChar("cset"));
	SET_STRING_ELT(infonames, 6, mkChar("fileno"));
	SET_STRING_ELT(infonames, 7, mkChar("addr"));
	SET_STRING_ELT(infonames, 8, mkChar("otype"));
	SET_STRING_ELT(infonames, 9, mkChar("rc"));
	SET_STRING_ELT(infonames, 10, mkChar("atime"));
	SET_STRING_ELT(infonames, 11, mkChar("mtime"));
	SET_STRING_ELT(infonames, 12, mkChar("ctime"));
	SET_STRING_ELT(infonames, 13, mkChar("btime"));
	SET_STRING_ELT(infonames, 14, mkChar("num_attrs"));
	SET_STRING_ELT(infonames, 15, mkChar("dclass"));
	SET_STRING_ELT(infonames, 16, mkChar("dtype"));
	SET_STRING_ELT(infonames, 17, mkChar("stype"));
	SET_STRING_ELT(infonames, 18, mkChar("rank"));
	SET_STRING_ELT(infonames, 19, mkChar("dim"));
	SET_NAMES(info, infonames);
	setAttrib(info, R_ClassSymbol, mkString("data.frame"));
	setAttrib(info, mkString("row.names"), ScalarInteger(1));
	UNPROTECT(1);
	
	SET_VECTOR_ELT(Rval,n,info);
	UNPROTECT(1);
      }
    }
    el = el->next;
    n = n + 1;
  }

  SET_NAMES(Rval, names);
  UNPROTECT(1);
  UNPROTECT(1);

  return(Rval);
}

SEXP _h5dump( SEXP _loc_id, SEXP _depth, SEXP _objecttype, SEXP _datasetinfo, SEXP _index_type, SEXP _order ) {
  hid_t loc_id = INTEGER(_loc_id)[0];
  opLinfoTree data;
  data.n = 0;
  data.maxdepth = INTEGER(_depth)[0];
  data.depth = 1;
  data.group = malloc(2*sizeof(char));
  strcpy(data.group, "/");
  data.objecttype = INTEGER(_objecttype)[0];
  data.showdatasetinfo = INTEGER(_datasetinfo)[0];
  data.insertAsChild = 0;
  data.first = NULL;
  data.last = NULL;
  data.index_type = INTEGER(_index_type)[0];
  data.order = INTEGER(_order)[0];
  hsize_t idx=0;
  /* printf("Start visit.\n"); */
  herr_t herr = H5Literate( loc_id, data.index_type, data.order, &idx, &opAddToLinfoTree, &data );

  SEXP Rval;
  Rval = getTree(data.first, &data, loc_id, 0);

  /* printf("End visit.\n"); */

  return Rval;  
}

