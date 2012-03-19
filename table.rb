ss = File.open(ARGV[0]) { |f| f.readlines }

results = Array.new
scores = Hash.new
totalBranches = Hash.new
totalCCBranches = Hash.new
totalPredicts = Hash.new

current = ""
ss.each do |s|
  next if s.strip == "" || s[0] == "*"
  if s[0..1] == "./" then
    current = s.strip.split(/[ \/]/)[-1]
    results.push(current)
  elsif s[0..3] == "1000" then
    scores[current] = s.split(" ")[-1]
  elsif s[6] == "b" then
    totalBranches[current] = s.split(" ")[-1]
  elsif s[6] == "c" then
    totalCCBranches[current] = s.split(" ")[-1]
  elsif s[6] == "p" then
    totalPredicts[current] = s.split(" ")[-1]
  else
    puts "Bad line! \"#{s}\""
  end
end

results.sort.each do |s|
  puts "#{s} & #{scores[s]} & #{totalBranches[s]} & #{totalCCBranches[s]} & #{totalPredicts[s]} \\\\"
end


