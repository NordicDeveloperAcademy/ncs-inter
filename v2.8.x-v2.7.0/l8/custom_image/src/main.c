/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <string.h>

static const char img_data[] = {
    #include "logo.file"
};

/** Display the Nordic Semiconductor logo with 1 second delay */
static void print_logo(void)
{
    printk("\n%s", img_data);
    k_sleep(K_MSEC(1000));
}

/** Print a banner with the name of the running simulation
 *
 * @param name The name of the simulation to display
 */
static void print_banner(const char *name)
{
    printk("-----------------------------------\n");
    printk(" Starting %s\n", name);
    printk("-----------------------------------\n");
}

/** Calculate the nth number in the Fibonacci sequence iteratively
 *
 * @param n The position in the sequence to calculate
 * @return The nth Fibonacci number
 */
static uint32_t calculate_fibonacci(uint32_t n) {
    uint32_t prev = 0, curr = 1, next;
    for (uint32_t i = 2; i <= n; i++) {
        next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}

/** Check if a number is prime using trial division
 *
 * @param n Number to check for primality
 * @return true if prime, false otherwise
 */
static bool is_prime(uint32_t n) {
    if (n <= 1) return false;
    for (uint32_t i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

/** Count all prime numbers up to a given limit
 *
 * @param limit Upper bound for prime counting
 * @return Number of primes found
 */
static uint32_t count_primes(uint32_t limit) {
    uint32_t count = 0;
    for (uint32_t i = 2; i < limit; i++) {
        if (is_prime(i)) count++;
    }
    return count;
}

/** Calculate steps in Collatz sequence until reaching 1
 *
 * @param n Starting number for sequence
 * @return Number of steps to reach 1
 */
static uint32_t collatz_sequence(uint32_t n) {
    uint32_t steps = 0;
    while (n != 1) {
        n = (n % 2 == 0) ? (n / 2) : (3 * n + 1);
        steps++;
    }
    return steps;
}

/** Find perfect numbers up to a given limit
 * A perfect number equals the sum of its proper divisors
 *
 * @param limit Upper bound for checking perfect numbers
 * @return Count of perfect numbers found
 */
static uint32_t perfect_number_check(uint32_t limit) {
    uint32_t count = 0;
    for (uint32_t n = 2; n < limit; n++) {
        uint32_t sum = 1;
        for (uint32_t i = 2; i * i <= n; i++) {
            if (n % i == 0) {
                sum += i;
                if (i != n / i) sum += n / i;
            }
        }
        if (sum == n) count++;
    }
    return count;
}

/** Run mathematical tasks with progress indication
 * Executes multiple computational tasks with increasing complexity
 *
 * @param level Current iteration level (affects input sizes)
 */
static void simulate_activity(uint32_t level)
{
    printk("\nRunning Mathematical tasks...\n");

    for (int step = 0; step < 10; step++) {
        // Progress indicator
        printk("\n[Task %d/10]\n", step + 1);
        printk("[");
        for (int i = 0; i < 20; i++) {
            printk(i < (step * 2) ? "=" : i == (step * 2) ? ">" : " ");
        }
        printk("] %d%%\n", (step + 1) * 10);

        // Mathematical tasks
        printk("Fibonacci(%d): %u\n", step + 10, calculate_fibonacci(step + 10));
        printk("Primes up to %d: %u\n", (step + 1) * 50, count_primes((step + 1) * 50));
        printk("Collatz steps for %d: %u\n", (step + 1) * 20, collatz_sequence((step + 1) * 20));
        printk("Perfect numbers up to %d: %u\n", (step + 1) * 100, perfect_number_check((step + 1) * 100));

        k_sleep(K_MSEC(200));
    }
}

int main(void)
{
    print_logo();

    if (strcmp(CONFIG_BOARD_TARGET, "nrf54l15dk/nrf54l15/cpuflpr") == 0) {
        print_banner("Fast, Lightweight Peripheral Processor (FLPR) Workload Simulation");
    } else if (strcmp(CONFIG_BOARD_TARGET, "nrf5340dk/nrf5340/cpunet") == 0) {
        print_banner("Network Core on the nRF5340 (CPUNET) Workload Simulation");
    }  

    uint32_t iteration = 0;

    while (1) {
        printk("\nStarting Benchmark Iteration: %d\n", iteration + 1);
        simulate_activity(iteration);
        iteration++;
        k_sleep(K_SECONDS(2));
        if(iteration > 3) {
          break;
        }
    }
    printk("Benchmark finished.\n");

    return 0;
}
