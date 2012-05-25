import sys
import glob
import csv
import numpy

num_iterations = int(sys.argv[1])

for folder in sys.argv[2:]:
  if (folder[-1] != '/'):
    folder += '/'
  file_names = glob.glob(folder + '*.csv*')
  result = []
  with open(file_names.pop(0),'rb') as first_file:
    reader = csv.reader(first_file)
    header = reader.next()
    measurement = header[-1]
    header = header[0:-1]
    for i in range(1, num_iterations+1):
      header.append(measurement + str(i))
    result.append(header)
    
    for row in reader:
      result.append(row)

  for file_name in file_names:
    with open(file_name,'rb') as new_file:
      reader = csv.reader(new_file)
      reader.next()
      i = 1
      for row in reader:
        result[i].append(row[-1])
        i+=1

  result[0].extend(["max", "min", "avg", "standard deviation"])
  for row in result[1:]:
    data = row[-1*num_iterations:]
    data = map(float, data)
    max_gflops = max(data)
    min_gflops = min(data)
    avg_gflops = round(numpy.average(data),3)
    std_dev = round(numpy.std(data),3)
    row.extend([max_gflops, min_gflops, avg_gflops, std_dev])

  print result
  with open(folder + "data.csv", 'wb') as out:
    csv.writer(out).writerows(result)


