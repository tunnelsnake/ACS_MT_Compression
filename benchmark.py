import matplotlib.pyplot as plt
import pickle
from subprocess import Popen, PIPE


def compress_command(input_path, output_path, compress_level, nThreads):
    return ['./a.out', 'compress', input_path, output_path, str(compress_level), str(nThreads)]


def decompress_command(input_path, output_path, compress_level, nThreads):
    return ['./a.out', 'decompress', input_path, output_path, str(compress_level), str(nThreads)]


test_files = ['res/alice_in_wonderland.txt']
nTrials = 100
max_compress_level = 20

print("This script will run all the way up to compression level 20.")
print("It writes its output to results.txt which can be later be graphed.")
print("Run it for as long as you feel comfortable, be sure to get at least a few levels.")

trials = []
if __name__ == "__main__":
    with open('results.txt', 'w') as f:
        f.write("nbThreads,compress_level,time_compress,time_decompress\n")

        for compress_level in range(1, max_compress_level):
            print(f"Starting Trials on Compression Level {compress_level}.")
            for nbThreads in range(1, 16):
                for trialNo in range(nTrials):
                    p = Popen(compress_command("res/alice_in_wonderland.txt",
                              "res/alice.comp", compress_level, nbThreads), stdout=PIPE, stderr=PIPE)

                    while p.poll() is None:
                        pass

                    stdout, stderr = p.communicate()
                    stderr = stderr.decode('ascii')
                    time_compress = int(stderr.split(" ")[1])
                    p = Popen(decompress_command("res/alice.comp", "res/alice.out",
                              compress_level, nbThreads), stdout=PIPE, stderr=PIPE)

                    while p.poll() is None:
                        pass

                    stdout, stderr = p.communicate()
                    stderr = stderr.decode('ascii')
                    time_decompress = int(stderr.split(" ")[1])
                    f.write(
                        f"{nbThreads},{compress_level},{time_compress},{time_decompress}\n")
