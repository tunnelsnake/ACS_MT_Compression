import pandas as pd
from matplotlib import pyplot as plt

df = pd.read_csv('results.txt')

# Graph avg compression time vs. number of threads
thread_num_dict = {}
max_compress_level = df["compress_level"].max() - 1
max_threads = df['nbThreads'].max()
for t in range(1, max_threads):
    time_compressed = df[(df['nbThreads'] == t)&(df['compress_level'] <= max_compress_level)]['time_compress']
    time_decompressed = df[(df['nbThreads'] == t)&(df['compress_level'] <= max_compress_level)]['time_decompress']
    thread_num_dict[t] = (time_compressed.mean(), time_decompressed.mean())

x = list(thread_num_dict.keys())
y = [thread_num_dict[i][0] for i in x]
z = [thread_num_dict[i][1] for i in x]
print(x)
print(y)
print(z)
# Number of threads vs. avg compress time



fig = plt.figure()
ax = fig.add_axes([0,0,1,1])
ax.grid(True, which='both', zorder = 0)

b = ax.bar(x,y, color='maroon', zorder=3)
plt.bar_label(b, x)
plt.show()
# df['age']=df.apply(lambda x: x['age']+3,axis=1)
