import sys
import glob
import csv
import numpy
import os

num_iterations = int(sys.argv[1])

results = {}
with open('data.csv','rb') as data:
  reader = csv.reader(data)
  header = reader.next()
  for row in reader:
    key = ",".join(row[0:-1])
    val = row[-1]
    if results.has_key(key):
      results[key].append(val)
    else:
      results[key] = [val]

with open('tmp.csv','wb') as tmp:
  for result in results.items():
    tmp.write(result[0] + "," +  ",".join(result[1])+"\n")

os.system("sort -t, -k 1,1 -k 2,2n -k 3,3n -k 4,4n -k 5,5n -k 6,6n tmp.csv -o tmp.csv")

measurement = header[-1]
header = header[0:-1]
header.extend(["avg", "max", "median", "min", "standard deviation"])
for i in range(1, num_iterations+1):
  header.append(measurement + "_" + str(i))

with open('data.csv','wb') as data:
  writer = csv.writer(data)
  writer.writerow(header)
  with open('tmp.csv','rb') as tmp:
    reader = csv.reader(tmp)
    for row in reader:
      key = row[:6]
      data = row[6:]
      data = map(float, data)
      max_gflops = round(max(data),3)
      min_gflops = round(min(data),3)
      median_gflops = round(numpy.median(data),3)
      avg_gflops = round(numpy.average(data),3)
      std_dev = round(numpy.std(data),3)
      key.extend([avg_gflops, max_gflops, median_gflops, min_gflops, std_dev])
      writer.writerow(key + data)

os.system("rm -f tmp.csv")
