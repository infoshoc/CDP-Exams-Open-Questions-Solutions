package Winter_2017_2018_A;

public class DoubleBarrier {
    int n;
    private boolean isEnter;
    private int currentNumberOfThreads;
    private boolean shouldWaitOnBarrier;

    public DoubleBarrier(int n) {
        this.n = n;
        isEnter = true;
        currentNumberOfThreads = 0;
        shouldWaitOnBarrier = true;
    }

    public void enter() {
        waitUntilIsEnter(true);

        waitOnBarrier();
    }

    public void leave() {
        waitUntilIsEnter(false);

        waitOnBarrier();
    }

    private synchronized void waitUntilIsEnter(boolean value) {
        while (value != isEnter) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }


    private synchronized void waitOnBarrier() {
        ++currentNumberOfThreads;

        if (n == currentNumberOfThreads) {
            shouldWaitOnBarrier = false;

            notifyAll(); //shouldWaitOnBarrier
        }
        else {
            while (shouldWaitOnBarrier) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        --currentNumberOfThreads;

        if (0 == currentNumberOfThreads) {
            shouldWaitOnBarrier = true;
            isEnter = !isEnter;

            notifyAll(); //waitUntilIsEnter
        }
    }

}
