"""Benchmark the installed extension against the checked-out reference files.

Run the same command with each build installed in its own environment:
    python benchmarks/benchmark_api.py --samples 7 --loops 3

Inputs and expected outputs are loaded before timing. One-shot timings include
construction, decompression and the first data() call; serialization timings
exclude decompression. Repeated reads are timed only if they return correct
bytes.
"""

import argparse
import json
from pathlib import Path
import platform
from statistics import median
import sys
from time import perf_counter
from timeit import repeat

import pyPublicDecompWT
from pyPublicDecompWT import xRITDecompress


def decode(payload):
    decoder = xRITDecompress()
    decoder.decompress(payload)
    return decoder


def one_shot(payloads):
    for payload in payloads:
        decode(payload).data()


def serialization_samples(payloads, samples, loops):
    timings = []
    for _ in range(samples):
        elapsed = 0.0
        for _ in range(loops):
            decoders = [decode(payload) for payload in payloads]
            start = perf_counter()
            outputs = [decoder.data() for decoder in decoders]
            elapsed += perf_counter() - start
            del outputs, decoders
        timings.append(elapsed / loops)
    return timings


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-dir", type=Path, default=Path(__file__).resolve().parents[1] / "test-data")
    parser.add_argument("--samples", type=int, default=7)
    parser.add_argument("--loops", type=int, default=3)
    args = parser.parse_args()
    if args.samples < 1 or args.loops < 1:
        parser.error("--samples and --loops must be positive")

    paths = sorted((args.data_dir / "compressed_ref").glob("*-C_"))
    if not paths:
        parser.error("No compressed reference files found; fetch the Git LFS fixtures.")
    payloads = [path.read_bytes() for path in paths]
    expected = [
        (args.data_dir / "uncompressed_ref" / path.name.replace("-C_", "-__")).read_bytes()
        for path in paths
    ]
    if any(data.startswith(b"version https://git-lfs.github.com/spec/v1") for data in payloads + expected):
        parser.error("Test data contains Git LFS pointers; fetch the actual fixtures.")

    # Also warm up the extension and verify each first result before measuring.
    decoders = [decode(payload) for payload in payloads]
    for path, decoder, reference in zip(paths, decoders, expected):
        if decoder.data() != reference:
            raise RuntimeError("Incorrect first result for " + path.name)

    repeated_correct = all(decoder.data() == reference for decoder, reference in zip(decoders, expected))
    repeated = {"correct": repeated_correct}
    if repeated_correct:
        calls = args.loops * 1000

        def read_cached():
            for decoder in decoders:
                decoder.data()

        repeated["median_ns_per_call"] = median(
            repeat(read_cached, repeat=args.samples, number=calls)
        ) * 1e9 / (calls * len(payloads))
    else:
        repeated["note"] = "Not timed: repeated data() returns incorrect output."
    del decoders

    one_shot_seconds = median(repeat(lambda: one_shot(payloads), repeat=args.samples, number=args.loops)) / args.loops
    serialization_seconds = median(serialization_samples(payloads, args.samples, args.loops))
    output_bytes = sum(map(len, expected))
    report = {
        "python": sys.version,
        "platform": platform.platform(),
        "extension": pyPublicDecompWT.__file__,
        "version": pyPublicDecompWT.__version__,
        "files": len(payloads),
        "input_bytes": sum(map(len, payloads)),
        "output_bytes": output_bytes,
        "samples": args.samples,
        "loops": args.loops,
        "one_shot": {
            "median_ms_per_file": one_shot_seconds * 1000 / len(payloads),
            "output_mib_per_second": output_bytes / (1024 ** 2) / one_shot_seconds,
        },
        "first_data": {
            "median_ms_per_file": serialization_seconds * 1000 / len(payloads),
        },
        "repeated_data": repeated,
    }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
