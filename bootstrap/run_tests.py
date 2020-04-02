#! /usr/bin/python3
import os
import sys
import subprocess
import shlex
import time

TEST_DIR = './tests/'
BINARY = './lang'
MAX_DIFF_LENGTH = 50
TIME_LIMIT = 2

def run_test(name):
    code_fname = os.path.join(TEST_DIR, name+'.lang')
    in_fname   = os.path.join(TEST_DIR, name+'.in')
    out_fname  = os.path.join(TEST_DIR, name+'.out')

    inp = open(in_fname)
    p = subprocess.Popen([BINARY, code_fname], universal_newlines=True,
            stdin=inp, stdout=subprocess.PIPE)
    st = time.time()
    tle = False
    print('st')
    while p.poll() is None:
        time.sleep(0.1)
        en = time.time()
        if en-st >= TIME_LIMIT:
            tle = True
            p.kill()
            break
    inp.close()

    if tle:
        print('[RUNNER] Test "{}" Failed!'.format(name), 'Time Limit Exceeded!')
        return False

    out, err = p.communicate()
    f = open(out_fname, 'r')
    exp = ''.join(f.readlines())
    f.close()
    if out != exp:
        print('[RUNNER] Test "{}" Failed!'.format(name), 'Wrong Answer!')
        print('[RUNNER]   Output:  ',
                '"' + out[:MAX_DIFF_LENGTH] + \
                        ('...' if len(out) > MAX_DIFF_LENGTH else '') + '"')
        print('[RUNNER]   Expected:',
                '"' + exp[:MAX_DIFF_LENGTH] + \
                        ('...' if len(exp) > MAX_DIFF_LENGTH else '') + '"')
        return False

    if p.returncode != 0:
        print('[RUNNER] Test "{}" Failed!'.format(name), 'Got non-zero return code!')
        return False

    return True


if len(sys.argv) > 1:
    name = sys.argv[1]
    if run_test(name):
        print('Test "{}" passed!'.format(name))

else:
    tests = []
    for fname in os.listdir(TEST_DIR):
        if fname.endswith('.lang'):
            tests.append(fname[:-5])

    results = [run_test(test) for test in tests]
    if all(results):
        print('All tests passed!')

    else:
        tot = sum(1 if x else 0 for x in results)
        print('Passed {} out of {} tests.'.format(tot, len(tests)))
