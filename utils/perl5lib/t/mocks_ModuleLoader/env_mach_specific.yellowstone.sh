#!/usr/bin/env sh -f 
#===============================================================================
# Automatically generated module settings for yellowstone
#===============================================================================

.  /glade/apps/opt/lmod/lmod/init/sh
module purge  
module load ncarenv/1.0 
module load ncarbinlibs/1.1 
module load perlmods 
module load gmake/4.1 
module load python 
module load all-python-libs 
if [ "$COMPILER" = "intel" ]
then
	module load intel/15.0.3
	module load mkl/11.1.2
	module load trilinos/11.10.2
	module load esmf
fi
if [ "$COMPILER" = "intel" ] && [ "$MPILIB" != "mpi-serial" ] && [ "$DEBUG" = "true" ]
then
	module load esmf-6.3.0rp1-defio-mpi-g
fi
if [ "$COMPILER" = "intel" ] && [ "$MPILIB" != "mpi-serial" ] && [ "$DEBUG" = "false" ]
then
	module load esmf-6.3.0rp1-defio-mpi-O
fi
if [ "$COMPILER" = "intel" ] && [ "$MPILIB" = "mpi-serial" ] && [ "$DEBUG" = "true" ]
then
	module load esmf-6.3.0rp1-defio-uni-g
fi
if [ "$COMPILER" = "intel" ] && [ "$MPILIB" = "mpi-serial" ] && [ "$DEBUG" = "false" ]
then
	module load  esmf-6.3.0rp1-defio-uni-O
fi
if [ "$COMPILER" = "pgi" ]
then
	module load pgi/15.1
fi
if [ "$COMPILER" = "gnu" ]
then
	module load gnu/5.1.0
fi
if [ "$MPILIB" = "mpi-serial" ]
then
	module load netcdf/4.3.3.1
fi
if [ "$MPILIB" != "mpi-serial" ]
then
	module load netcdf-mpi/4.3.3.1
	module load pnetcdf/1.6.0
fi
module load ncarcompilers/1.0 
module load cmake/2.8.10.2 
export OMP_STACKSIZE=256M
export MP_LABELIO=yes
export MP_INFOLEVEL=2
export MP_SHARED_MEMORY=yes
export MP_EUILIB=us
export MP_MPILIB=$MPILIB
export MP_STDOUTMODE=unordered
export MP_RC_USE_LMC=yes
ulimit -s unlimited
ulimit -c unlimited
