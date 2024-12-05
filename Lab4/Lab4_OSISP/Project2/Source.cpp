#include <vector>
#include <string>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

typedef boost::mutex Fork;
typedef boost::shared_ptr< Fork > ForkPtr;
typedef boost::lock_guard< Fork > ForkLock;

#define MIN_WAIT_TIME 100
#define NUM_MEALS     10
#define MAX_JITTER    50

template< typename Stream >
class AtomicLogger {
public:

    AtomicLogger(Stream& stream) :
        m_mutex(),
        m_stream(stream)
    {
    }

    void log(const std::string& str) {
        boost::mutex::scoped_lock lock(m_mutex);
        m_stream << str << std::endl;
    }

private:
    mutable boost::mutex m_mutex;
    Stream& m_stream;
};
typedef AtomicLogger< std::ostream > AtomicLoggerOstream;
typedef boost::shared_ptr< AtomicLoggerOstream > AtomicLoggerOstreamPtr;

class Philosopher {
public:

    Philosopher(
        const std::string& name,
        ForkPtr fork_left,
        ForkPtr fork_right,
        AtomicLoggerOstreamPtr p_logger) :
        m_name(name),
        m_continue(true),
        mp_fork_left(fork_left),
        mp_fork_right(fork_right),
        m_thread(boost::thread(boost::bind(&Philosopher::thread_func,
            this,
            &m_continue,
            mp_fork_left,
            mp_fork_right))),
        m_meals_left(NUM_MEALS),
        mp_logger(p_logger),
        m_successful_forks(0),
        m_unsuccessful_forks(0),
        m_waiting_time(0)
    {
    }

    ~Philosopher() {
        done_dining();
        wait_for_cmplt();
    }

    void done_dining() { m_continue = false; }

    void wait_for_cmplt() { m_thread.join(); }

private:
    inline bool can_grab_fork(ForkPtr& p_fork) { return p_fork->try_lock(); }

    void thread_func(volatile bool* p_continue, ForkPtr fork_left, ForkPtr fork_right) {
        bool failed_to_grab_fork = false;

        while (p_continue && m_meals_left) {
            mp_logger->log(boost::str(boost::format("%1% is thinking") % this->m_name));
            wait();
            mp_logger->log(boost::str(boost::format("%1% is hungry") % this->m_name));

            const boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
            if (can_grab_fork(fork_left)) {
                ForkLock lock_left(*fork_left, boost::adopt_lock);
                if (can_grab_fork(fork_right)) {
                    ForkLock lock_right(*fork_right, boost::adopt_lock);
                    mp_logger->log(boost::str(boost::format("%1% is eating (%2%)...") % m_name % m_meals_left));
                    wait();
                    ++m_successful_forks;
                    --m_meals_left;
                }
                else {
                    failed_to_grab_fork = true;
                }
            }
            else {
                failed_to_grab_fork = true;
            }

            const boost::posix_time::ptime end_time = boost::posix_time::microsec_clock::local_time();
            m_waiting_time += (end_time - start_time).total_milliseconds();

            if (failed_to_grab_fork) {
                mp_logger->log(boost::str(boost::format("%1% couldn't get forks; waiting...") % m_name));
                failed_to_grab_fork = false;
                ++m_unsuccessful_forks;
                wait();
            }
        }

        mp_logger->log(boost::str(boost::format("%1% is done dining") % m_name));
    }

    inline void wait() {
        wait(MIN_WAIT_TIME + (std::rand() % MAX_JITTER));
    }

    inline void wait(boost::uint32_t time_in_ms) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(time_in_ms));
    }
public:
    std::string m_name;
    volatile bool m_continue;
    ForkPtr mp_fork_left;
    ForkPtr mp_fork_right;
    boost::thread m_thread;
    boost::uint32_t m_meals_left;
    AtomicLoggerOstreamPtr mp_logger;
    boost::uint64_t m_successful_forks;
    boost::uint64_t m_unsuccessful_forks;
    boost::uint64_t m_waiting_time;
};
typedef boost::shared_ptr< Philosopher > PhilosopherPtr;

int main() {
    const int N = 5;
    std::string names[] = { "Aristotle", "Spinoza", "Russell", "Kant", "Plato" };

    std::vector< PhilosopherPtr > philosophers;
    philosophers.reserve(N);

    AtomicLoggerOstreamPtr p_logger(new AtomicLoggerOstream(std::cout));

    std::vector< ForkPtr > forks;
    forks.reserve(N);
    for (int i = 0; i < N; ++i) {
        forks.push_back(ForkPtr(new Fork()));
    }

    for (int i = 0; i < N; ++i) {
        philosophers.push_back(PhilosopherPtr(
            new Philosopher(names[i], forks[i], forks[(i + 1) % N], p_logger)));
    }

    for (int i = 0; i < N; ++i) {
        philosophers[i]->wait_for_cmplt();
    }

    boost::uint64_t total_successful_forks = 0;
    boost::uint64_t total_unsuccessful_forks = 0;
    boost::uint64_t total_waiting_time = 0;

    for (int i = 0; i < N; ++i) {
        total_successful_forks += philosophers[i]->m_successful_forks;
        total_unsuccessful_forks += philosophers[i]->m_unsuccessful_forks;
        total_waiting_time += philosophers[i]->m_waiting_time;
        std::cout << philosophers[i]->m_name << ": Philosopher successful forks: " << philosophers[i]->m_successful_forks << std::endl;
        std::cout << philosophers[i]->m_name << ": Philosopher unsuccessful forks: " << philosophers[i]->m_unsuccessful_forks << std::endl;
        std::cout << philosophers[i]->m_name << ": Philosopher waiting time: " << philosophers[i]->m_waiting_time << std::endl;
    }

    p_logger->log(boost::str(boost::format("Total successful forks: %1%") % total_successful_forks));
    p_logger->log(boost::str(boost::format("Total unsuccessful forks: %1%") % total_unsuccessful_forks));
    p_logger->log(boost::str(boost::format("Total waiting time: %1% ms") % total_waiting_time));

    p_logger->log("Everyone is done dining.");

    return 0;
}
