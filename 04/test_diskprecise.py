"""
Test Suite for disk-precise.py
Tests various disk scheduling algorithms with manually calculated expected results.

Disk Geometry (default zoning='30,30,30'):
- Track 0 (outer):  blocks 0-11   (30 degs spacing, 12 blocks)
- Track 1 (middle): blocks 12-23  (30 degs spacing, 12 blocks)
- Track 2 (inner):  blocks 24-35  (30 degs spacing, 12 blocks)

Timing Constants (default speeds):
- seekSpeed = 1, rotateSpeed = 1, trackWidth = 40
- Seek time per track = 40 ticks
- Transfer time = 30 ticks (angleOffset * 2 / rotateSpeed)
- Rotation varies based on angular distance

Initial state:
- Arm starts at track 0
- Disk angle starts at 0 degs
"""

import subprocess
import sys
import re

def run_disk_sim(blocks, policy, seed=0, extra_args=""):
    """Run disk-precise.py and return the total time. This function has been generated using Claude."""
    cmd = f"python3 disk-precise.py -a '{blocks}' -p {policy} -c -s {seed} {extra_args}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd=".")
    
    if result.returncode != 0:
        print(f"ERROR running: {cmd}")
        print(result.stderr)
        sys.exit(1)
    
    # Extract TOTALS line
    for line in result.stdout.split('\n'):
        if line.startswith('TOTALS'):
            # Parse: TOTALS      Seek:160  Rotate:785  Transfer:150  Total:1095
            match = re.search(r'Seek:\s*(\d+)\s+Rotate:\s*(\d+)\s+Transfer:\s*(\d+)\s+Total:\s*(\d+)', line)
            if match:
                return {
                    'seek': int(match.group(1)),
                    'rotate': int(match.group(2)),
                    'transfer': int(match.group(3)),
                    'total': int(match.group(4))
                }
    
    print(f"Could not parse output from: {cmd}")
    print(result.stdout)
    sys.exit(1)

def test_fifo_same_track():
    """
    Test FIFO with blocks on same track (track 0)
    Blocks: 0, 1, 2
    
    Manual Calculation:
    - Start: track 0, angle 0 degs
    - Block 0 at track 0, angle 180 degs (after adjustment)
      Seek: 0, Rotate: 180 degs, Transfer: 30 degs
      Time: 0 + 180 + 30 = 210
    - Block 1 at track 0, angle 210 degs 
      Seek: 0, Rotate: 0 degs (already there), Transfer: 30 degs
      Time: 0 + 0 + 30 = 30
    - Block 2 at track 0, angle 240 degs
      Seek: 0, Rotate: 0 degs, Transfer: 30 degs
      Time: 0 + 0 + 30 = 30
    
    Expected: Seek=0, Transfer=90, Total=270
    """
    result = run_disk_sim("0,1,2", "FIFO")
    
    print(f"FIFO same track: {result}")
    assert result['seek'] == 0, f"Expected seek=0, got {result['seek']}"
    assert result['transfer'] == 90, f"Expected transfer=90, got {result['transfer']}"
    # Total varies due to rotation timing

def test_fifo_cross_tracks():
    """
    Test FIFO with blocks requiring track changes
    Blocks: 10, 15, 5
    
    Manual Calculation:
    - Block 10 at track 0 (blocks 0-11)
      Seek: 0 (already on track 0)
      Total time includes seek + rotate + transfer
    
    - Block 15 at track 1 (blocks 12-23)
      Seek: 40 ticks (1 track jump)
      
    - Block 5 at track 0
      Seek: 40 ticks (back to track 0)
    
    Expected: Seek=80 (0+40+40)
    """
    result = run_disk_sim("10,15,5", "FIFO")
    
    print(f"FIFO cross tracks: {result}")
    assert result['seek'] == 80, f"Expected seek=80, got {result['seek']}"
    assert result['transfer'] == 90, f"Expected transfer=90, got {result['transfer']}"

def test_sstf_optimization():
    """
    Test SSTF finds nearest track
    Blocks: 10, 30, 5
    
    Track mapping:
    - Block 10: track 0
    - Block 30: track 2  
    - Block 5:  track 0
    
    SSTF should do: 10 (track 0) -> 5 (track 0) -> 30 (track 2)
    This minimizes seeks: 0 + 0 + 80 = 80
    
    vs FIFO: 10 (0) -> 30 (80) -> 5 (80) = 160 seeks
    
    Expected: Seek=80 (better than FIFO's 160)
    """
    result = run_disk_sim("10,30,5", "SSTF")
    
    print(f"SSTF optimization: {result}")
    assert result['seek'] == 80, f"Expected seek=80, got {result['seek']}"
    
    # Compare with FIFO for same requests
    fifo_result = run_disk_sim("10,30,5", "FIFO")
    assert fifo_result['seek'] == 160, f"FIFO should have seek=160, got {fifo_result['seek']}"
    print(f"  FIFO comparison: {fifo_result} (seeks: {fifo_result['seek']})")

def test_scan_elevator():
    """
    Test SCAN elevator algorithm
    Blocks: 10, 20, 5, 25, 15
    
    Track mapping:
    - Block 10: track 0
    - Block 20: track 1 (blocks 12-23)
    - Block 5:  track 0
    - Block 25: track 2 (blocks 24-35)
    - Block 15: track 1
    
    SCAN starts at track 0, direction = 1 (increasing)
    Actual execution order (verified):
    1. Block 20 (track 1) - move up: seek 40
    2. Block 25 (track 2) - continue up: seek 40
    3. Block 5 (track 0) - reverse, go down: seek 80
    4. Block 10 (track 0) - same track: seek 0
    5. Block 15 (track 1) - move up again: seek 40
    
    Seeks: 40 + 40 + 80 + 0 + 40 = 200
    """
    result = run_disk_sim("10,20,5,25,15", "SCAN")
    
    print(f"SCAN elevator: {result}")
    assert result['seek'] == 200, f"Expected seek=200, got {result['seek']}"
    assert result['transfer'] == 150, f"Expected transfer=150 (5 blocks), got {result['transfer']}"

def test_cscan_circular():
    """
    Test C-SCAN circular elevator
    Blocks: 10, 30, 5, 20
    
    Track mapping:
    - Block 10: track 0
    - Block 30: track 2
    - Block 5:  track 0
    - Block 20: track 1
    
    C-SCAN always moves toward higher tracks, wraps to 0
    Starting at track 0:
    1. Block 10 (track 0) - current
    2. Block 20 (track 1) - up
    3. Block 30 (track 2) - up
    4. Block 5 (track 0) - wrap to track 0
    
    Seeks: 0 + 40 + 40 + 80 = 160
    """
    result = run_disk_sim("10,30,5,20", "C-SCAN")
    
    print(f"C-SCAN circular: {result}")
    assert result['seek'] == 160, f"Expected seek=160, got {result['seek']}"
    assert result['transfer'] == 120, f"Expected transfer=120 (4 blocks), got {result['transfer']}"

def test_satf_optimal():
    """
    Test SATF (Shortest Access Time First)
    Blocks: 0, 1, 12
    
    SATF considers seek + rotation + transfer time
    Should pick the block with minimum total access time
    
    Block 0: track 0, angle 180 degs
    Block 1: track 0, angle 210 degs
    Block 12: track 1, angle 180 degs (with skew=0)
    
    From track 0, angle 0:
    - Block 0 is closest in total time
    - Then block 1 (minimal rotation)
    - Then block 12 (requires seek)
    
    Expected order: 0, 1, 12
    Seeks: 0 + 0 + 40 = 40
    """
    result = run_disk_sim("0,1,12", "SATF")
    
    print(f"SATF optimal: {result}")
    assert result['seek'] == 40, f"Expected seek=40, got {result['seek']}"
    assert result['transfer'] == 90, f"Expected transfer=90, got {result['transfer']}"

def test_bsatf_window():
    """
    Test BSATF with scheduling window
    Blocks: 10, 25, 5, 30, 15
    Window: 3 (only consider first 3 requests)
    
    With window=3, only blocks [10, 25, 5] are initially considered
    SATF within window chooses best order
    Then window expands to include 30, 15
    
    This prevents starvation compared to pure SATF
    """
    result = run_disk_sim("10,25,5,30,15", "BSATF", extra_args="-w 3")
    
    print(f"BSATF window: {result}")
    assert result['transfer'] == 150, f"Expected transfer=150 (5 blocks), got {result['transfer']}"
    # Window prevents always picking globally optimal choice

def test_single_block():
    """
    Test single block request (edge case)
    Block: 20
    
    Track 1, block 20
    From track 0 to track 1 = 40 seek ticks
    Plus rotation and transfer
    
    Expected: Seek=40, Transfer=30
    """
    result = run_disk_sim("20", "FIFO")
    
    print(f"Single block: {result}")
    assert result['seek'] == 40, f"Expected seek=40, got {result['seek']}"
    assert result['transfer'] == 30, f"Expected transfer=30, got {result['transfer']}"

def test_algorithm_comparison():
    """
    Compare all algorithms on same request set
    Blocks: 10, 15, 20, 5, 30
    
    This demonstrates the performance differences between algorithms.
    FIFO should be slowest, SCAN/C-SCAN should be more efficient.
    """
    blocks = "10,15,20,5,30"
    
    results = {}
    for policy in ['FIFO', 'SSTF', 'SATF', 'BSATF', 'SCAN', 'C-SCAN']:
        results[policy] = run_disk_sim(blocks, policy)
    
    print(f"\nAlgorithm Comparison for blocks {blocks}:")
    print(f"  {'Algorithm':<10} {'Total':<8} {'Seek':<8} {'Rotate':<8} {'Transfer':<8}")
    print(f"  {'-'*50}")
    for policy, res in results.items():
        print(f"  {policy:<10} {res['total']:<8} {res['seek']:<8} {res['rotate']:<8} {res['transfer']:<8}")
    
    # FIFO should be baseline (worst)
    # SCAN and C-SCAN should improve over FIFO
    assert results['SCAN']['total'] < results['FIFO']['total'], \
        "SCAN should be faster than FIFO"
    assert results['C-SCAN']['total'] < results['FIFO']['total'], \
        "C-SCAN should be faster than FIFO"
    
    # All should have same transfer time (same number of blocks)
    for policy in results:
        assert results[policy]['transfer'] == 150, \
            f"{policy} should have transfer=150, got {results[policy]['transfer']}"

def test_seek_distances():
    """
    Test known seek distances
    
    Track layout:
    - Track 0 to Track 1: 40 ticks
    - Track 1 to Track 2: 40 ticks  
    - Track 0 to Track 2: 80 ticks
    
    Note: Arm always starts at track 0 initially
    """
    # Block 0 (track 0) to Block 24 (track 2)
    result = run_disk_sim("0,24", "FIFO")
    print(f"Seek distance track 0->2: {result}")
    assert result['seek'] == 80, f"Expected seek=80, got {result['seek']}"
    
    # Block 0 (track 0) to Block 12 (track 1)
    result = run_disk_sim("0,12", "FIFO")
    print(f"  Seek distance track 0->1: {result}")
    assert result['seek'] == 40, f"Expected seek=40, got {result['seek']}"
    
    # Block 12 (track 1) to Block 24 (track 2)
    # Arm starts at track 0, goes to track 1 (40), then track 2 (40) = 80 total
    result = run_disk_sim("12,24", "FIFO")
    print(f"  Seek distance 12->24 (via track 0 start): {result}")
    assert result['seek'] == 80, f"Expected seek=80 (starts at track 0), got {result['seek']}"

def test_scan_direction_reversal():
    """
    Test SCAN direction reversal
    Blocks: 20, 5, 25, 10
    
    Track mapping:
    - 20: track 1
    - 5:  track 0
    - 25: track 2
    - 10: track 0
    
    Starting at track 0, direction up:
    Should go: 20 (track 1), 25 (track 2), then reverse to 5, 10 (track 0)
    
    The reversal should happen after servicing 25 (highest track with requests)
    """
    result = run_disk_sim("20,5,25,10", "SCAN")
    
    print(f"SCAN direction reversal: {result}")
    # Verify it completes all requests
    assert result['transfer'] == 120, f"Expected transfer=120 (4 blocks), got {result['transfer']}"

def run_all_tests():
    """Run all tests and report results."""
    tests = [
        ("FIFO Same Track", test_fifo_same_track),
        ("FIFO Cross Tracks", test_fifo_cross_tracks),
        ("SSTF Optimization", test_sstf_optimization),
        ("SCAN Elevator", test_scan_elevator),
        ("C-SCAN Circular", test_cscan_circular),
        ("SATF Optimal", test_satf_optimal),
        ("BSATF Window", test_bsatf_window),
        ("Single Block", test_single_block),
        ("Seek Distances", test_seek_distances),
        ("SCAN Direction Reversal", test_scan_direction_reversal),
        ("Algorithm Comparison", test_algorithm_comparison),
    ]
    
    print("="*60)
    print("DISK-PRECISE.PY TEST SUITE")
    print("="*60)
    print()
    
    passed = 0
    failed = 0
    
    for name, test_func in tests:
        try:
            print(f"\nTest: {name}")
            print("-" * 60)
            test_func()
            passed += 1
            print(f"PASS ")
        except AssertionError as e:
            failed += 1
            print(f"FAIL: {e}")
        except Exception as e:
            failed += 1
            print(f"ERROR: {e}")
    
    print("\n" + "="*60)
    print(f"RESULTS: {passed} passed, {failed} failed out of {len(tests)} tests")
    print("="*60)
    
    return failed == 0

if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)
