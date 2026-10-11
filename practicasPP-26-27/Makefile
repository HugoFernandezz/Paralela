# OS Name (Linux or Darwin)
OSUPPER = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
OSLOWER = $(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])

# Flags to detect 32-bit or 64-bit OS platform
OS_SIZE = $(shell uname -m | sed -e "s/i.86/32/" -e "s/x86_64/64/")
OS_ARCH = $(shell uname -m | sed -e "s/i386/i686/")


# These flags will override any settings
ifeq ($(i386),1)
	OS_SIZE = 32
	OS_ARCH = i686
endif

ifeq ($(x86_64),1)
	OS_SIZE = 64
	OS_ARCH = x86_64
endif

# Flags to detect either a Linux system (linux) or Mac OSX (darwin)
DARWIN = $(strip $(findstring DARWIN, $(OSUPPER)))

# Location of the CUDA Toolkit binaries and libraries
CUDA_PATH       ?= /usr/local/cuda
CUDA_INC_PATH   ?= $(CUDA_PATH)/include
CUDA_BIN_PATH   ?= $(CUDA_PATH)/bin
ifneq ($(DARWIN),)
  CUDA_LIB_PATH  ?= $(CUDA_PATH)/lib
else
  ifeq ($(OS_SIZE),32)
    CUDA_LIB_PATH  ?= $(CUDA_PATH)/lib
  else
    CUDA_LIB_PATH  ?= $(CUDA_PATH)/lib64
  endif
endif

# Common binaries
NVCC            ?= $(CUDA_BIN_PATH)/nvcc
GCC             ?= g++

EXTRA_NVCCFLAGS ?= 
EXTRA_LDFLAGS   ?=  

# CUDA code generation flags 37 50 52 60 61
#GENCODE_SM10    := -gencode arch=compute_10,code=sm_10
#GENCODE_SM20    := -gencode arch=compute_20,code=sm_20
#GENCODE_SM35    := -gencode arch=compute_35,code=sm_35
#GENCODE_SM37    := -gencode arch=compute_37,code=sm_37
#GENCODE_SM50    := -gencode arch=compute_50,code=sm_50
#GENCODE_SM52    := -gencode arch=compute_52,code=sm_52
#GENCODE_SM60    := -gencode arch=compute_60,code=sm_60
#GENCODE_SM61    := -gencode arch=compute_61,code=sm_61
#GENCODE_SM70    := -gencode arch=compute_70,code=sm_70
GENCODE_SM75    := -gencode arch=compute_75,code=sm_75
#GENCODE_SM80    := -gencode arch=compute_80,code=sm_80
#GENCODE_SM86    := -gencode arch=compute_86,code=sm_86
GENCODE_FLAGS   := $(GENCODE_SM75) --cudart=shared

# OS-specific build flags
ifneq ($(DARWIN),) 
      LDFLAGS   := -Xlinker -rpath -L$(CUDA_LIB_PATH) -lstdc++ -fopenmp
      CCFLAGS   := -arch $(OS_ARCH) 
else
  ifeq ($(OS_SIZE),32)
      LDFLAGS   := -lstdc++ -fopenmp
      CCFLAGS   := -m32
  else
      LDFLAGS   := -L$(CUDA_LIB_PATH) -lcurand -lcublas -lcudart -lstdc++ -fopenmp
      CCFLAGS   := -m64
  endif
endif

# OS-architecture specific flags
ifeq ($(OS_SIZE),32)
      NVCCFLAGS := -m32
else
      NVCCFLAGS := -m64
endif

# Debug build flags
ifeq ($(dbg),1)
      CCFLAGS   += 
      NVCCFLAGS += 
      TARGET    := debug
else
      # -O2 igual para todas las versiones (decision 1 de DECISIONES.md).
      # En nvcc, -O solo afecta al codigo de la CPU: los kernels se optimizan siempre.
      CCFLAGS   += -O2  #-ffast-math #-march=native -std=c++17
      NVCCFLAGS += -O2
      TARGET    := release
endif

# -Wall en lugar del -w original, que ocultaba todos los avisos.
# nvcc no entiende -Wall: se lo pasa al compilador de la CPU con -Xcompiler.
WARNFLAGS      := -Wall
NVCC_WARNFLAGS := -Xcompiler -Wall


# Common includes and paths for CUDA
INCLUDES      := -I$(CUDA_INC_PATH)  

# Target rules
all: build

build: fichero_ejecutable

wtime.o: wtime.cpp
	$(GCC) $(CCFLAGS) $(EXTRA_CCFLAGS) -o $@ -c $(WARNFLAGS) -fopenmp $<

fichero_common.o: fichero_common.cpp
	$(GCC) $(CCFLAGS) $(EXTRA_CCFLAGS) -o $@ -c $(WARNFLAGS) -fopenmp $<

fichero_omp.o: fichero_omp.cpp
	$(GCC) $(CCFLAGS) $(EXTRA_CCFLAGS) -o $@ -c $(WARNFLAGS) -fopenmp $<

fichero_gpu.o: fichero_gpu.cu
	$(NVCC) $(CCFLAGS) $(GENCODE_FLAGS) $(EXTRA_CCFLAGS) $(INCLUDES) -o $@ -c $(NVCC_WARNFLAGS) -Xptxas -dlcm=ca $<

fichero_cpu.o: fichero_cpu.cpp
	$(GCC) $(CCFLAGS) $(INCLUDES) $(EXTRA_CCFLAGS) -o $@ -c $(WARNFLAGS) -fopenmp $<

main.o: main.cpp
	$(GCC) $(CCFLAGS) $(EXTRA_CCFLAGS) -o $@ -c $(WARNFLAGS) -fopenmp $<

fichero_ejecutable: main.o wtime.o fichero_common.o fichero_cpu.o fichero_omp.o fichero_gpu.o
	$(GCC) $(CCFLAGS) -o $@ $+ $(LDFLAGS) $(EXTRA_LDFLAGS)

run: build
	./fichero_ejecutable

clean:
	rm -f fichero_ejecutable *.o log

