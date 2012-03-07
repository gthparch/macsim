#!/usr/bin/perl -w
# (tri pho 07-11-2011 rewrite for component macsim)

use File::stat;
use Time::localtime;

#search for *.param.def files
@files = <../def/*.stat.def>;
if (0 == scalar(@files)) {
	print "no such files:  *.param.def\n";
	exit();
}

#check for updates
$allstats = "../src/all_stats.cc";
$allstath = "../src/all_stats.h";
$statsEnums = "../src/statsEnums.h";
if (-e $allstats and -e $statsEnums and -e $allstath) {
	$timestamp = stat($allstats)->mtime;
	if (stat($statsEnums)->mtime < $timestamp) {
		$timestamp = stat($statsEnums)->mtime;
	}

	$def_file_changed = 0;

	foreach $file (@files) {
		if ($timestamp < stat($file)->mtime) {
			$def_file_changed = 1;
			last;
		}
	}

	if ($def_file_changed == 0) {
		exit 0;
	}
}

# status running
print "./statgen.pl\n";

#Text to write to file
@StatisticsEnum = (); #global enum elements
@coreStatsEnum  = (); #per core enum elements

@allstats_constructor = ();
@allstats_deconstruct = ();
@allstats_init_globalInit       = ();
@allstats_init_globalSetup      = ();
@allstats_init_globalCollection = ();
@allstats_init_coreInit         = ();
@allstats_init_coreSetup        = ();
@allstats_init_coreCollection   = ();
@allstats_declare     = ();

#used for uniquness
%declaredStats = ();

$distributionActive = 0;
$activeDistributionName = "";
$distributionVariable = "";

$StatisticsOutputFileName = "";
$CurrentDefFile = "";

$StatsContainer = "m_ProcessorStats->globalStats()";


#per core stats, nagesh on apr-07-2009
push (@coreStatsEnum, "CORE_STATS_START = PER_CORE_STATS_ENUM_START,\n");
$isPerCoreStat = 0;
$isPerCoreDistActive = 0;
$coreStatsContainer = "m_coreStatsTemplate";
#per core stats, nagesh on apr-07-2009


#StatisticsEnum
#all_stats.h Header
#all_stats.cc Source
foreach $file (@files) {
	$temp = $file;
	$temp =~ s/^\.\///;

	$CurrentDefFile = $temp;
  $distributionActive = 0;
  $isPerCoreDistActive = 0;

	processFile($file);
}




# Create files in case of there is at least one change.
##### all_stats.h  #####
open(ALLSTATH, ">$allstath") || die("Can not open file $allstath\n");
	
	print ALLSTATH "#ifndef _ALL_STATS_C_INCLUDED_\n";
	print ALLSTATH "#define _ALL_STATS_C_INCLUDED_\n";
	
	print ALLSTATH "#include \"statistics.h\"\n\n";
	
  print ALLSTATH "///////////////////////////////////////////////////////////////////////////////////////////////\n";
  print ALLSTATH "/// \\brief knob variables holder\n";
  print ALLSTATH "///////////////////////////////////////////////////////////////////////////////////////////////\n";
	print ALLSTATH "class all_stats_c {\n";
	  print ALLSTATH "\tpublic:\n";	
			#prototypes
      print ALLSTATH "\t\t/**\n";
      print ALLSTATH "\t\t * Constructor\n";
      print ALLSTATH "\t\t */\n";
			print ALLSTATH "\t\tall_stats_c(ProcessorStatistics\* procStat);\n\n";

      print ALLSTATH "\t\t/**\n";
      print ALLSTATH "\t\t * Constructor\n";
      print ALLSTATH "\t\t */\n";
			print ALLSTATH "\t\t~all_stats_c();\n\n";

      print ALLSTATH "\t\t/**\n";
      print ALLSTATH "\t\t * Constructor\n";
      print ALLSTATH "\t\t */\n";
			print ALLSTATH "\t\tvoid initialize(ProcessorStatistics\*, CoreStatistics\*);\n\n";
		
			#member stat variables
			for $declares (@allstats_declare) {
				print ALLSTATH "\t\t".$declares;
			}
	
	print ALLSTATH "};\n\n";
	
	print ALLSTATH "#endif //ALL_STATS_C_INCLUDED\n";
	
close(ALLSTATH);

##### all_stats.cc #####
open(ALLSTATS, ">$allstats") || die("Can not open file $allstats\n");
	
	print ALLSTATS "#include \"all_stats.h\"\n";
	print ALLSTATS "#include \"statsEnums.h\"\n\n";
	
	#constructor
	print ALLSTATS "all_stats_c::all_stats_c(ProcessorStatistics\* procStat) {\n";
		for $construct (@allstats_constructor) {print ALLSTATS "\t".$construct;}
	print ALLSTATS "}\n\n";
	
	#destructor
	print ALLSTATS "all_stats_c::\~all_stats_c() {\n";
		for $deconst (@allstats_deconstruct) {print ALLSTATS "\t".$deconst;}
	print ALLSTATS "}\n\n";
	
	#initialization and configuration
	print ALLSTATS "void all_stats_c::initialize(ProcessorStatistics\* m_ProcessorStats, CoreStatistics\* m_coreStatsTemplate) {\n";
		for $initLine (@allstats_init_globalInit)       {print ALLSTATS "\t".$initLine;}
		print ALLSTATS "\n\n";
		for $initLine (@allstats_init_globalSetup)      {print ALLSTATS "\t".$initLine;}
		print ALLSTATS "\n\n";
		for $initLine (@allstats_init_globalCollection) {print ALLSTATS "\t".$initLine;}
		print ALLSTATS "\n\n";
		for $initLine (@allstats_init_coreInit)         {print ALLSTATS "\t".$initLine;}
		print ALLSTATS "\n\n";
		for $initLine (@allstats_init_coreSetup)        {print ALLSTATS "\t".$initLine;}
		print ALLSTATS "\n\n";
		for $initLine (@allstats_init_coreCollection)   {print ALLSTATS "\t".$initLine;}
	print ALLSTATS "}\n";
	
close (ALLSTATS);

##### statsEnums.h #####
	open(ENUMDEF, ">$statsEnums") || die("Can not open file $statsEnums\n");
	
	print ENUMDEF "#ifndef StatsEnums_Included\n";
	print ENUMDEF "#define StatsEnums_Included\n\n";
	
	#per core stats, nagesh on apr-07-2009
	$allCoreStatsEnumStart = 5000;
	print ENUMDEF "#define PER_CORE_STATS_ENUM_START $allCoreStatsEnumStart\n";
	print ENUMDEF "#define PER_CORE_STATS_ENUM_FIRST " . ($allCoreStatsEnumStart+1) . "\n\n";
	#per core stats, nagesh on apr-07-2009
	
	print ENUMDEF "enum StatisticsEnum\n{\n";
	
	foreach $defLine (@StatisticsEnum) {print ENUMDEF $defLine;}
	print ENUMDEF "\n\n";
	foreach $defLine (@coreStatsEnum) {print ENUMDEF $defLine;}
	
	print ENUMDEF "\tStatisticsEnumEnd\n};\n";
	print ENUMDEF "#endif //StatsEnums_Included\n";
	
	close(ENUMDEF);



##################### SUBROUTINES #######################

sub processFile
{
	my ($file) = @_;

	@temp_list = split(/\//, $file);
	$headerfile = "../src/$temp_list[-1]";
	$headerfile =~ s/\.def/\.h/;

	$StatisticsOutputFileName = $temp_list[-1];
	#	$StatisticsOutputFileName = $file;
	$StatisticsOutputFileName =~ s/^\.\///;
	$StatisticsOutputFileName =~ s/\.def/\.out/;

	#Open PARAM.DEF file
	open(PARAMFILE, "<$file") || die("Can not open file $file\n");
	@theWholeText = <PARAMFILE>;
	close(PARAMFILE);


	### Sanitize
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


	### Parse
	open(PARAMFILE, "<$newName") || die("Can not open file $newName\n");

	@param_lines = <PARAMFILE>;
  
	push (@allstats_constructor, "\n\t// ============= $file =============\n");
	push (@allstats_declare,     "\n\t\t// ============= $file =============\n");
	foreach $param_line (@param_lines) {
		processLine($param_line);
	}
	close(PARAMFILE);
	unlink($newName);
}


#####################################################################
sub processLine
{
	my ($param_line) = @_;

	if($param_line =~ /^\s*DEF_STAT/) {
		#print "Global Stat:     $param_line \n";
	}

	if($param_line =~ /^\s*BEGIN_DISTRIBUTION/) {
		$param_line =~ s/\s*BEGIN_DISTRIBUTION\s*<\s*//;
		$param_line =~ s/\s*>\s*$//;

		$distributionName = $param_line;
		$distributionName =~ s/\s*//g;

		$distributionName = $distributionName . "_Dist";

		$isDistributionActive = 1;

		#print "$distributionName\n";
	}

	if($param_line =~ /^\s*END_DISTRIBUTION/) {
		$isDistributionActive = 0;
		$distributionName = "";
	}

	if($param_line =~ /\s*DEF_STAT\s*\(/) {
		$param_line =~ s/\s*DEF_STAT\s*\(\s*//;

		$param_line =~ s/\s*\)\s*$//;
		@elements = split(/,/, $param_line);

		$StatName = $elements[0];
		$StatName =~ s/\s*//g;

		$StatType = $elements[1];
		$StatType =~ s/\s*//g;

		$field_1 = $elements[0];
		$field_1 =~ s/\s*//g;

		$field_2 = $elements[1];
		$field_2 =~ s/\s*//g;

		$field_4 = $elements[3];
		if (defined($field_4)) {
			$field_4 =~ s/\s*//g;
			if ($field_4 eq "PER_CORE") {
				$isPerCoreStat = 1;
			}
			else {
				$isPerCoreStat = 0;
			}
		}
		else {
			$isPerCoreStat = 0;
		}

		if ($field_2 eq "DIST") {
			if ($isPerCoreStat eq 1) {
				if ($isPerCoreDistActive eq 1) {
					declareDistributionMember($StatName);
					defineDistributionMember($StatName);
				}
				else {
					$activeDistributionName = $StatName;
					declareDistribution($StatName);
					defineDistribution($StatName);

					# this is the first member of the distribution, so declare it and define it
					declareDistributionMember($StatName);
					defineDistributionMember($StatName);
				}
				$isPerCoreDistActive = not($isPerCoreDistActive);
			}
			else {
				if ($isPerCoreDistActive eq 1) {
					Die ("$StatName should be a per core stat\n");
				}

				if($distributionActive eq 1) {
					declareDistributionMember($StatName);
					defineDistributionMember($StatName);
				}
				else {
					$activeDistributionName = $StatName;
					declareDistribution($StatName);
					defineDistribution($StatName);

					# this is the first member of the distribution, so declare it and define it
					declareDistributionMember($StatName);
					defineDistributionMember($StatName);
				}

				$distributionActive = not($distributionActive);
			}
		}
		else {
			if ($isPerCoreStat eq 1) {
				if ($isPerCoreDistActive eq 1) {
					declareDistributionMember($StatName);
					defineDistributionMember($StatName);
				}
				else {
					if(($StatType eq "RATIO") or ($StatType eq "PERCENT")) {
						$field_3 = $elements[2];
						$field_3 =~ s/\s*//g;

						declareRatioStatistic($StatName, $StatType, $field_3);
						defineRatioStatistic($StatName, $StatType, $field_3);
					}
					else {
						declareStatistic($StatName, $StatType);
						defineStatistic($StatName, $StatType);
					}
				}
			}
			else {
				if ($isPerCoreDistActive eq 1) {
					Die("$StatName should be per core stat\n");
				}

				if($distributionActive eq 1) {
					declareDistributionMember($StatName);
					defineDistributionMember($StatName);
				}
				else {
					if (($StatType eq "RATIO") or ($StatType eq "PERCENT")) {
						$field_3 = $elements[2];
						$field_3 =~ s/\s*//g;

						declareRatioStatistic($StatName, $StatType, $field_3);
						defineRatioStatistic($StatName, $StatType, $field_3);
					}
					else {
						declareStatistic($StatName, $StatType);
						defineStatistic($StatName, $StatType);
					}
				}
			}
		}


		#$className = $field_2 . "_Stat";

		#print "$className $StatName(\"$StatName\")\n";
	}
}

#####################################################################
sub declareDistribution
{
	my $StatName = shift;
	$distributionVariable = "m_DIST_$StatName";

	#     ensureUniqueness("DIST_$StatName");
	
	push (@allstats_declare, "DIST_Stat\* $distributionVariable;\n");

	if ($isPerCoreStat eq 1) {
		push (@allstats_init_coreCollection, "$coreStatsContainer->addDistribution($distributionVariable);\n");
	}
	else {
		push (@allstats_init_globalCollection, "$StatsContainer->addDistribution($distributionVariable);\n");
	}

}

#####################################################################
sub defineDistribution
{
	my $StatName = shift;
	$distributionVariable = "m_DIST_$StatName";

	push (@allstats_constructor, "$distributionVariable = new DIST_Stat(\"$StatName\", \"$StatisticsOutputFileName\", $StatName, procStat);\n");
	push (@allstats_deconstruct, "delete $distributionVariable;\n");
}

#####################################################################
sub declareDistributionMember
{
	my $StatName = shift;
	my $variableName = "m_$StatName";

	ensureUniqueness($StatName);

	if ($isPerCoreStat eq 1) {
		push (@coreStatsEnum, "$StatName,\n");
	}
	else {
		push (@StatisticsEnum, "$StatName,\n");
	}

	push (@allstats_declare, "COUNT_Stat\* $variableName;\n");
}

#####################################################################
sub defineDistributionMember
{
	my $StatName = shift;
	my $variableName = "m_$StatName";

	push (@allstats_constructor, "$variableName = (COUNT_Stat*) new DISTMember_Stat(\"$StatName\", \"$StatisticsOutputFileName\", $StatName, $activeDistributionName);\n");
	push (@allstats_deconstruct, "delete $variableName;\n");
	if ($isPerCoreStat eq 1) {
		push (@allstats_init_coreInit, "$coreStatsContainer->addStatistic($variableName);\n");
		push (@allstats_init_coreSetup, "$distributionVariable->addMember($StatName);\n");
	}
	else {
		push (@allstats_init_globalInit, "$StatsContainer->addStatistic($variableName);\n");
		push (@allstats_init_globalSetup, "$distributionVariable->addMember($StatName);\n");
	}
	# 	printf "$definition\n";
}

#####################################################################
sub declareRatioStatistic
{
	$StatName = shift;
	$StatType = shift;
	$Ratio = shift;

	ensureUniqueness($StatName);

	if ($isPerCoreStat eq 1) {
		push (@coreStatsEnum, "$StatName,\n");
	}
	else {
		push (@StatisticsEnum, "$StatName,\n");
	}

	my $variableName = "m_$StatName";
	push (@allstats_declare, $StatType."_Stat\* $variableName;\n");
}

#####################################################################
sub defineRatioStatistic
{
	$StatName = shift;
	$StatType = shift;
	$Ratio = shift;

	my $variableName = "m_$StatName";

	push (@allstats_constructor, "$variableName = new ".$StatType."_Stat(\"$StatName\",  \"$StatisticsOutputFileName\", $StatName, $Ratio, procStat);\n");
	push (@allstats_deconstruct, "delete $variableName;\n");
	if ($isPerCoreStat eq 1) {
		push (@allstats_init_coreInit, "$coreStatsContainer->addStatistic($variableName);\n");
	}
	else {
		push (@allstats_init_globalInit, "$StatsContainer->addStatistic($variableName);\n");
	}
	# 	printf "$definition\n";
}

#####################################################################
sub declareStatistic
{
	$StatName = shift;
	$StatType = shift;

	my $variableName = "m_$StatName";

	ensureUniqueness($StatName);

	if ($isPerCoreStat eq 1) {
		push (@coreStatsEnum, "$StatName,\n");
	}
	else {
		push (@StatisticsEnum, "$StatName,\n");
	}

	push (@allstats_declare, $StatType."_Stat\* $variableName;\n");

}

#####################################################################
sub defineStatistic
{
	$StatName = shift;
	$StatType = shift;

	my $variableName = "m_$StatName";

	push (@allstats_constructor, "$variableName = new ".$StatType."_Stat(\"$StatName\", \"$StatisticsOutputFileName\", $StatName);\n");
	push (@allstats_deconstruct, "delete $variableName;\n");
	if ($isPerCoreStat eq 1) {
		push (@allstats_init_coreInit, "$coreStatsContainer->addStatistic($variableName);\n");
	}
	else {
		push (@allstats_init_globalInit, "$StatsContainer->addStatistic($variableName);\n");
	}

	# 	printf "$definition\n";

}

#####################################################################
sub ensureUniqueness
{
	my $StatName = shift;

if($declaredStats{$StatName}) {
	print "ERROR:  $StatName : Duplicate definition in $CurrentDefFile\n";
	print "\t$StatName Already Defined  in $declaredStats{$StatName}\n";
	die("\n Please make sure no duplication happens in Statistics Names!\n\n Exiting!\n\n");
}
else {
	#         print "adding entry $StatName to the hash\n";
	$declaredStats{$StatName} = $CurrentDefFile;
}

}
