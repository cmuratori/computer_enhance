# To build the extension:
#   ruby extconf.rb && make
#
# To test the extension using RSpec:
#   gem install rspec
#   rspec .

require 'mkmf'

single_ids = []
ops = []
File.foreach('../sim86_shared.h') do |line|
  if line.match(/^ +Op_([a-z]*),$/)
    ops << $1
  end
end
File.foreach('sim86.cpp') do |line|
  if line.match(/^ID id_([a-z0-9_]*);/)
    single_ids << $1
  end
end

gen = File.open('generated.inc', 'w')
gen.puts <<END
static void init_ids() {
  for (int i = 0; i < Op_Count; i++) { id_ops[i] = Qnil; }
END
ops.each do |op|
  gen.puts "  id_ops[Op_#{op}] = rb_intern(\"#{op}\");"
end
single_ids.each do |name|
  gen.puts "  id_#{name} = rb_intern(\"#{name}\");"
end
gen.puts '}'


$CXXFLAGS += "-I.. "
create_makefile("sim86")
