#include <pio.h>
#include <pio_internal.h>
#define versno 2000

void piodie(const char *msg,const char *fname, const int line){
  fprintf(stderr,"Abort with message %s in file %s at line %d\n",msg,fname,line);
#ifdef MPI_SERIAL
  abort();
#else
  MPI_Abort(MPI_COMM_WORLD, -1);
#endif
}


int check_netcdf(file_desc_t *file,int status, const char *fname, const int line){
  iosystem_desc_t *ios;
  int ierr;

  ios = file->iosystem;
  ierr = PIO_NOERR;

  switch(file->iotype){
#ifdef _NETCDF
#ifdef _NETCDF4
  case PIO_IOTYPE_NETCDF4P:
  case PIO_IOTYPE_NETCDF4C:
#endif
  case PIO_IOTYPE_NETCDF:
    if(ios->iomaster){
      if(status != NC_NOERR)	
	fprintf(stderr,"NETCDF ERROR: %s %s %d\n",nc_strerror(status),fname,line);
    }
    if(ios->error_handler == PIO_INTERNAL_ERROR){
      if(status != NC_NOERR)	
	MPI_Abort(MPI_COMM_WORLD,status);
      // abort
    }else if(ios->error_handler==PIO_BCAST_ERROR){
      ierr =MPI_Bcast(&status, 1, MPI_INTEGER, ios->ioroot, ios->my_comm);
    }
    break;
#endif
#ifdef _PNETCDF
  case PIO_IOTYPE_PNETCDF:
      if(ios->iomaster){
	if(status != NC_NOERR)
	  fprintf(stderr,"PNETCDF ERROR: %s %s %d\n",ncmpi_strerror(status),fname,line);
      }
      if(ios->error_handler == PIO_INTERNAL_ERROR){
	if(status != NC_NOERR)	
	  MPI_Abort(MPI_COMM_WORLD,status);
	// abort
      }else if(ios->error_handler==PIO_BCAST_ERROR){
	ierr = MPI_Bcast(&status, 1, MPI_INTEGER, ios->ioroot, ios->my_comm);
      }
    break;
#endif
  default:
    ierr = iotype_error(file->iotype,__FILE__,__LINE__);
  } 
  return status;
}

int iotype_error(const int iotype, const char *fname, const int line){
    fprintf(stderr, "ERROR: iotype %d not defined in build %s %d\n", iotype, fname,line);
    return PIO_NOERR;
}

io_desc_t *malloc_iodesc(const int piotype, const int ndims)
{
  io_desc_t *iodesc;
  iodesc = (io_desc_t *) malloc(sizeof(io_desc_t));

  if(iodesc == NULL)
    fprintf(stderr,"ERROR: allocation error \n");

  switch(piotype){
  case PIO_REAL:			
    iodesc->basetype=MPI_FLOAT;
    break;
  case PIO_DOUBLE:
    iodesc->basetype=MPI_DOUBLE;
    break;
  case PIO_CHAR:
    iodesc->basetype=MPI_CHAR;
    break;
  case PIO_INT:   
  default:
    iodesc->basetype = MPI_INTEGER;
    break;
  }    
  iodesc->rearranger = 0;
  iodesc->rfrom = NULL;
  iodesc->scount = NULL;
  iodesc->rtype = NULL;
  iodesc->stype = NULL;
  iodesc->sindex = NULL;
  iodesc->rindex = NULL;
  iodesc->rcount = NULL;
  iodesc->ioid=-1;
  iodesc->llen=0;
  iodesc->ndims = ndims;
  iodesc->start = (PIO_Offset *) calloc(ndims, sizeof(PIO_Offset));
  iodesc->count = (PIO_Offset *) calloc(ndims, sizeof(PIO_Offset));

  return iodesc;
}

int PIOc_freedecomp(int iosysid, int ioid)
{
  iosystem_desc_t *ios;
  io_desc_t *iodesc;
  ios = pio_get_iosystem_from_id(iosysid);
  if(ios == NULL)
    return PIO_EBADID;

  iodesc = pio_get_iodesc_from_id(ioid);
  if(iodesc == NULL)
    return PIO_EBADID;

  if(iodesc->rfrom != NULL)
    free(iodesc->rfrom);
  if(iodesc->scount != NULL)
    free(iodesc->scount);
  if(iodesc->rcount != NULL)
    free(iodesc->rcount);
  if(iodesc->sindex != NULL)
    free(iodesc->sindex);
  if(iodesc->rindex != NULL)
    free(iodesc->rindex);

  if(iodesc->rtype != NULL)
    free(iodesc->rtype);
  if(iodesc->stype != NULL)
    free(iodesc->stype);
  if(iodesc->start != NULL)
    free(iodesc->start);
  if(iodesc->count != NULL)
    free(iodesc->count);

  return pio_delete_iodesc_from_list(ioid);


}






int PIOc_writemap(const char file[], PIO_Offset maplen, const PIO_Offset map[], const MPI_Comm comm)
{
  int npes, myrank;
  PIO_Offset *nmaplen;
  MPI_Status status;
  int i;
  PIO_Offset *nmap;

  MPI_Comm_size(comm, &npes);
  MPI_Comm_rank(comm, &myrank);
  if(myrank==0)
    nmaplen = (PIO_Offset *) malloc(npes*sizeof(PIO_Offset));
  else
    nmaplen = NULL;

  printf("maplen[%d] = %ld\n",myrank,maplen);


  MPI_Gather(&maplen, 1, PIO_OFFSET, nmaplen, 1, PIO_OFFSET, 0, comm);

  if(myrank==0){
    FILE *fp;

    for(i=0;i<npes;i++)
      printf("nmaplen[%d] = %ld\n",i,nmaplen[i]);


    fp = fopen( file, "w");
    if(fp==NULL){
      fprintf(stderr,"Failed to open file %s to write\n",file);
      return PIO_EIO;
    }
    fprintf(fp,"version %d npes %d\n",versno, npes);
    fprintf(fp,"0 %ld\n",nmaplen[0]);
    for( i=0;i<nmaplen[0];i++)
      fprintf(fp,"%ld ",map[i]);
    
    for( i=1; i<npes;i++){
      nmap = (PIO_Offset *) malloc(nmaplen[i] * sizeof(PIO_Offset));

      MPI_Send(&i, 1, MPI_INT, i, npes+i, comm);
      MPI_Recv(nmap, nmaplen+i, PIO_OFFSET, i, i, comm, &status);

      fprintf(fp,"%d %ld\n",i,nmaplen[i]);
      for(int j=0;j<nmaplen[i];j++)
	fprintf(fp,"%ld ",nmap[j]);
      fprintf(fp,"\n");

      free(nmap);
      
    }
    fclose(fp);
  }else{
    MPI_Recv(&i, 1, MPI_INT, 0, npes+myrank, comm, &status);
    MPI_Send(map, maplen, PIO_OFFSET, 0, myrank, comm);
  }
  return PIO_NOERR;
}

int PIOc_writemap_from_f90(const char file[], const PIO_Offset maplen, const PIO_Offset map[], const int f90_comm)
{
  // printf("%s %d %s %ld\n",__FILE__,__LINE__,file,maplen);
  return(PIOc_writemap(file, maplen, map, MPI_Comm_f2c(f90_comm)));
}