# cmake/CompilerOptions.cmake
#
# Call apply_compiler_options(my_target) on any target you own.
# Third-party targets should NOT be passed to this function.

function(apply_compiler_options target)
	target_compile_options(${target} PRIVATE
		# MSVC-specific flags (the generator expression filters to MSVC only)
		$<$<CXX_COMPILER_ID:MSVC>:
			/W4		# High warning level (but not quite /Wall which is extremely noisy)
			/permissive-	# Enforce standard conformance, reject MSVC extensions
		>
	)

	# Debug-only flags
	target_compile_options(${target} PRIVATE
		$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:
			/Od		# Disable optimisations
			/RTC1		# Runtime checks: stack corruption + uninitialised variables
		>
	)

	# Release-only flags
	target_compile_options(${target} PRIVATE
		$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:
			/O2		# Maximise optimisations
		>
	)
endfunction()
