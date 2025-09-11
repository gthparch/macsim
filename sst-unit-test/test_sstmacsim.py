from sst_unittest import *
from sst_unittest_support import *
import threading

################################################################################
# Code to support a single instance module initialize, must be called in setUp method

module_init = 0
module_sema = threading.Semaphore()

def initializeTestModule_SingleInstance(class_inst):
    global module_init
    global module_sema

    module_sema.acquire()
    if module_init != 1:
        try:
            # Put your single instance Init Code Here
            pass
        except:
            pass
        module_init = 1
    module_sema.release()

################################################################################

class TestSSTExternalElement(SSTTestCase):

    def initializeClass(self, testName):
        super().initializeClass(testName)
        # Put test-based setup code here. It is called before testing starts.

    def setUp(self):
        super().setUp()
        initializeTestModule_SingleInstance(self)
        # Put test-based setup code here. It is called once before every test.

    def tearDown(self):
        # Put test-based teardown code here. It is called once after every test.
        super().tearDown()

    #####
    @unittest.skipIf(testing_check_get_num_ranks() > 1, "Test skipped if ranks > 1 - single component in config")
    @unittest.skipIf(testing_check_get_num_threads() > 1, "Test skipped if threads > 1 - single component in config")
    def test_sst_external_element_001(self):
        self.sst_external_element_test_template("macsimComponent-test-001")

    #####
    def sst_external_element_test_template(self, testcase):
        test_path = self.get_testsuite_dir()
        outdir = self.get_test_output_run_dir()
        tmpdir = self.get_test_output_tmp_dir()

        testDataFileName = "sst_external_element_{0}".format(testcase)

        sdlfile = "{0}/{1}.py".format(test_path, testcase)
        reffile = "{0}/refFiles/{1}.out".format(test_path, testcase)
        outfile = "{0}/{1}.out".format(outdir, testDataFileName)
        errfile = "{0}/{1}.err".format(outdir, testDataFileName)
        mpioutfiles = "{0}/{1}.testfile".format(outdir, testDataFileName)

        self.run_sst(sdlfile, outfile, errfile, mpi_out_files=mpioutfiles)

        testing_remove_component_warning_from_file(outfile)

        # Perform the tests
        self.assertFalse(os_test_file(errfile, "-s"), "Test {0} has non-empty Error File {1}".format(testDataFileName, errfile))

        cmp_result = testing_compare_sorted_diff(testcase, outfile, reffile)
        if not cmp_result:
            diffdata = testing_get_diff_data(testcase)
            log_failure(diffdata)
        self.assertTrue(cmp_result, "Sorted output file {0} does not match sorted reference file {1}".format(outfile, reffile))
