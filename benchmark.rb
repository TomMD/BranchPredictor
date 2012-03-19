#! /usr/bin/env ruby

Dir.entries("traces").each do |s|
  next unless File.file?("traces/" + s)
  cmd = "./predictor traces/#{File.basename(s, ".bz2")}"
  File.open(ARGV[0],"a") { |f| f.puts "\n#{cmd}\n" }
  puts cmd
  `#{cmd} >> #{ARGV[0]}`
end
