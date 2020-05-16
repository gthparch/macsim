#!/usr/bin/perl -w
#knobgen.pl for use with componentMACSIM

use File::stat;
use File::Basename;
use Time::localtime;

##### VARIABLES #####
#all parameter definitions
my @files;


$working_dir = dirname(__FILE__);

#output filepaths
my $allknobs_c = "$working_dir/all_knobs.cc";
my $allknobs_h = "$working_dir/all_knobs.h";

# lines in source
my @headerDeclares = ();
my @constructors = ();
my @registerCalls = ();
my @deconstructs = ();

##### START #####

### Search for parameter definitions
@files = <$working_dir/*.def>;
if (0 == scalar(@files)) {
  print "no such files:  *.param.def\n";
  exit();
}


### check to see if all_knobs sources need to be updated
if (-e $allknobs_c) {
  my $timestamp = stat($allknobs_c)->mtime;
  my $def_file_changed = 0;

  foreach my $file (@files) {
    if ($timestamp < stat($file)->mtime) {
      $def_file_changed = 1;
      last;
    }
  }

  if ($def_file_changed == 0) {
    exit 0;
  }
}

### status running
print "./knobgen.pl\n";


### die if can't access all_knobs.cc/h
open(ALLKNOBS_C, ">$allknobs_c") || die("Can not open file $allknobs_c\n");
open(ALLKNOBS_H, ">$allknobs_h") || die("Can not open file $allknobs_h\n");



foreach my $file (@files) {
  parseFile($file);
}

writeSource();
writeHeader();


##### Subroutines #####

### PARSE FILE SUB ###
my @names = ();
my @values = ();

sub parseFile
{
  my ($file) = @_;
  @names = ();
  @values = ();

  my @temp_list = split(/\//, $file);

  print "processing file: $file\n";

  ### LOAD FILE ###
  open(PARAMFILE, "<$file") || die("Can not open file $file\n");
  @theWholeText = <PARAMFILE>;
  close(PARAMFILE);

  ### SANITIZE TEXT ###
  my $newText;

  foreach $textLine (@theWholeText) {
    $newText = $newText . $textLine;
  }

  #remove C style comments
  $newText =~ s |/\*.*?.\*/||gsx;

  # remove C++ style comments
  $newText =~ s|//.*||g;

  $newName = $file . "_withoutcomments";
  open(NEWPARAMFILE, ">$newName") || die("Can not open file $newName\n");
  print NEWPARAMFILE $newText;
  close(NEWPARAMFILE);




  ### READ SANITIZED PARAM DEF ###
  open(PARAMFILE, "<$newName") || die("Can not open file $newName\n");

  @param_lines = <PARAMFILE>;

  push(@headerDeclares, "\n\n\t// =========== $file ===========\n");
  push(@constructors,   "\n\n\t// =========== $file ===========\n");
  push(@registerCalls,  "\n\n\t// =========== $file ===========\n");
  foreach $param_line (@param_lines) {
    processLine($param_line);
  }

  close(PARAMFILE);

  unlink($newName);
}


################################################################################
sub processLine
{
  my ($param_line) = @_;
  my $parentName = "";


  if($param_line =~ /\s*param/) {
    $param_line =~ s/^\s*param\s*<\s*//;
    $param_line =~ s/\s*>\s*$//;

    @elements = split(/,/, $param_line);

    $KnobName = "KNOB_".$elements[0];
    $paramfileentry = $elements[1];
    $datatype = $elements[2];
    $defaultvalue = $elements[3];


    $KnobName =~ s/\s*//g;
    $paramfileentry =~ s/\s*//g;
    $datatype =~ s/\s*//g;
    $defaultvalue =~ s/\s*//g;

    if ($defaultvalue =~ m/KNOB_.*/i) {
      $parentName = lc($defaultvalue);
      for ($i = 0; $i <= $#names; $i++) {
        if (lc($names[$i]) eq $parentName) {
          $defaultvalue = $values[$i];
          $parentName = substr ($parentName, 5);
          last;
        }
      }
    }

    push(@names, "knob_".$elements[0]);
    push(@values, $defaultvalue);

    
    push(@headerDeclares, "KnobTemplate< $datatype >* $KnobName;\n");
    push(@registerCalls,  "container->insertKnob( $KnobName );\n");
    push(@deconstructs,   "delete $KnobName;\n");
    if ($elements[2] =~ /\s*string\s*/) {
      if ($parentName ne "") {
        push(@constructors, "$KnobName = new KnobTemplate< $datatype > (\"$paramfileentry\", \"$defaultvalue\", \"$parentName\");\n");
      }
      else {
        push(@constructors, "$KnobName = new KnobTemplate< $datatype > (\"$paramfileentry\", \"$defaultvalue\");\n");
      }
    }
    else {
      if ($parentName ne "") {
        push(@constructors, "$KnobName = new KnobTemplate< $datatype > (\"$paramfileentry\", $defaultvalue, \"$parentName\");\n");
      }
      else {
        push(@constructors, "$KnobName = new KnobTemplate< $datatype > (\"$paramfileentry\", $defaultvalue);\n");
      }
    }
  }
}
################################################################################


################################################################################
sub writeSource
{
  print ALLKNOBS_C "#include \"all_knobs.h\"\n\n";
  
  #constructor
  print ALLKNOBS_C "all_knobs_c::all_knobs_c() {\n";
  foreach $constructor (@constructors) {
    print ALLKNOBS_C "\t$constructor";
  }
  print ALLKNOBS_C "}\n\n";
  
  
  #deconstructor
  print ALLKNOBS_C "all_knobs_c::~all_knobs_c() {\n";
  foreach $deconstructor (@deconstructs) {
    print ALLKNOBS_C "\t$deconstructor";
  }
  print ALLKNOBS_C "}\n\n";
  
  
  #registerKnob function
  print ALLKNOBS_C "void all_knobs_c::registerKnobs(KnobsContainer *container) {\n";
  foreach $registerCall (@registerCalls) {
    print ALLKNOBS_C "\t$registerCall";
  }
  print ALLKNOBS_C "}\n\n";
  
}
################################################################################

################################################################################
sub writeHeader
{
  print ALLKNOBS_H "#ifndef __ALL_KNOBS_H_INCLUDED__\n";
  print ALLKNOBS_H "#define __ALL_KNOBS_H_INCLUDED__\n\n";
  
  print ALLKNOBS_H "#include \"global_types.h\"\n";
  print ALLKNOBS_H "#include \"knob.h\"\n\n";
  
  print ALLKNOBS_H "class all_knobs_c {\n";
  
  print ALLKNOBS_H "\tpublic:\n";
  print ALLKNOBS_H "\t\tall_knobs_c();\n";
  print ALLKNOBS_H "\t\t~all_knobs_c();\n\n";
  print ALLKNOBS_H "\t\tvoid registerKnobs(KnobsContainer *container);\n\n";
  
  print ALLKNOBS_H "\tpublic:\n";
  foreach $vardef (@headerDeclares) {
    print ALLKNOBS_H "\t\t$vardef";
  }
  
  print ALLKNOBS_H "\n};\n";
  print ALLKNOBS_H "#endif //__ALL_KNOBS_H_INCLUDED__\n";
  
}
################################################################################
