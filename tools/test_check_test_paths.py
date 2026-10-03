#!/usr/bin/env python3
"""Regression tests for wrong CTest intersections and missing classifications."""

import unittest

from check_test_paths import inventory_tests, validate_inventory, validate_selection


class TestPathTests(unittest.TestCase):
    def setUp(self):
        self.tests = {
            "a1": {"water-a1", "fast", "property", "fast-water-a1"},
            "a1-study": {"water-a1", "slow", "research"},
            "b2": {"water-b2", "fast", "property", "fast-water-b2"},
        }

    def test_valid_partition(self):
        self.assertEqual(validate_inventory(self.tests), [])
        self.assertEqual(validate_selection(self.tests, {"a1"}, "water-a1"), [])

    def test_module_only_rejected(self):
        self.assertTrue(validate_selection(self.tests, {"a1", "a1-study"}, "water-a1"))

    def test_or_filter_rejected(self):
        self.assertTrue(validate_selection(self.tests, set(self.tests), "water-a1"))

    def test_wrong_module_rejected(self):
        self.assertTrue(validate_selection(self.tests, {"b2"}, "water-a1"))

    def test_missing_case_rejected(self):
        self.assertTrue(validate_selection(self.tests, set(), "water-a1"))

    def test_unlabelled_and_mixed_tiers_rejected(self):
        self.tests["a1"] = set()
        self.assertTrue(validate_inventory(self.tests))
        self.tests["a1"] = {"water-a1", "fast", "slow", "property", "fast-water-a1"}
        self.assertTrue(validate_inventory(self.tests))

    def test_research_cannot_be_hidden_in_fast(self):
        self.tests["a1"].add("research")
        self.assertTrue(validate_inventory(self.tests))

    def test_stale_alias_rejected(self):
        self.tests["a1-study"].add("fast-water-a1")
        self.assertTrue(validate_inventory(self.tests))

    def test_slow_performance_kind_is_valid(self):
        self.tests["benchmark"] = {"core", "slow", "performance"}
        self.assertEqual(validate_inventory(self.tests), [])

    def test_shared_case_belongs_to_both_modules(self):
        labels = self.tests["a1"]
        labels.update({"water-common", "water-b1", "fast-water-common", "fast-water-b1"})
        self.assertEqual(validate_inventory(self.tests), [])
        self.assertEqual(validate_selection(self.tests, {"a1"}, "water-b1"), [])

    def test_disabled_module_distinct_from_missing_fast_coverage(self):
        self.assertEqual(validate_selection(self.tests, set(), "water-d1"), [])
        self.tests["d1-study"] = {"water-d1", "slow", "research"}
        self.assertTrue(validate_selection(self.tests, set(), "water-d1"))

    def test_empty_and_duplicate_inventory_rejected(self):
        self.assertTrue(validate_inventory({}))
        with self.assertRaises(ValueError):
            inventory_tests({"tests": [{"name": "same", "properties": []}] * 2})


if __name__ == "__main__":
    unittest.main()
