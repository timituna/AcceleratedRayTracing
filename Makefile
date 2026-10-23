CUDA_PATH     ?= /usr/local/cuda
HOST_COMPILER  = g++
NVCC           = $(CUDA_PATH)/bin/nvcc -ccbin $(HOST_COMPILER)

# select one of these for Debug vs. Release
#NVCC_DBG       = -g -G
NVCC_DBG       = -lineinfo -O3

NVCCFLAGS      = $(NVCC_DBG) -m64 --use_fast_math
# RTX 4080 Laptop (Ada Lovelace, compute capability 8.9)
# The compute_89 PTX entry lets the driver JIT for future GPUs too.
GENCODE_FLAGS  = -gencode arch=compute_89,code=sm_89 \
                 -gencode arch=compute_89,code=compute_89

TOOL          ?= memcheck

.PHONY: clean profile_basic profile_metrics memcheck

cudart: cudart.o
	$(NVCC) $(NVCCFLAGS) $(GENCODE_FLAGS) -o cudart cudart.o

cudart.o: main.cu
	$(NVCC) $(NVCCFLAGS) $(GENCODE_FLAGS) -o cudart.o -c main.cu

# Usage: make memcheck            		(out-of-bounds / misaligned access, leaks)
#        make memcheck TOOL=racecheck   (shared-memory race conditions)
#        make memcheck TOOL=initcheck   (uninitialized global memory reads)
#        make memcheck TOOL=synccheck   (invalid __syncthreads usage)
memcheck: cudart
	compute-sanitizer --tool $(TOOL) --leak-check full --show-backtrace yes \
		--log-file $(TOOL).txt ./cudart > /dev/null
	@echo "=== $(TOOL) report ($(TOOL).txt) ==="
	@cat $(TOOL).txt

out.ppm: cudart
	rm -f out.ppm
	./cudart > out.ppm

out.jpg: out.ppm
	rm -f out.jpg
	ppmtojpeg out.ppm > out.jpg

# Nsight Systems: timeline + kernel/API summary (replaces nvprof basic mode)
profile_basic: cudart
	nsys profile --stats=true --force-overwrite=true -o profile_basic \
		sh -c './cudart > out.ppm'

# Nsight Compute: per-kernel metrics (replaces nvprof --metrics)
# Writes its report to a file so it doesn't corrupt the PPM on stdout.
# List available metrics with: ncu --query-metrics
profile_metrics: cudart
	ncu --log-file profile_metrics.txt \
		--metrics sm__warps_active.avg.pct_of_peak_sustained_active,smsp__inst_executed.sum,sm__sass_thread_inst_executed_op_fadd_pred_on.sum,sm__sass_thread_inst_executed_op_fmul_pred_on.sum,sm__sass_thread_inst_executed_op_ffma_pred_on.sum,sm__sass_thread_inst_executed_op_dadd_pred_on.sum,sm__sass_thread_inst_executed_op_dmul_pred_on.sum,sm__sass_thread_inst_executed_op_dfma_pred_on.sum,sm__sass_thread_inst_executed_op_integer_pred_on.sum \
		./cudart > out.ppm

clean:
	rm -f cudart cudart.o cudart_dbg out.ppm out.jpg profile_basic.* profile_metrics.txt \
		memcheck.txt racecheck.txt initcheck.txt synccheck.txt
