package Winter_2014_2015_A;

public class IntArray {
    private static final int N = 10000;
    private int x[] = new int [N];
    private int y[] = new int [N];
    private int readersNumber[] = new int [N];
    private Object readersNumberLock[] = new Object [N];

    private boolean legalIndex(int p) {
        return 0 <= p && p < N;
    }

    public IntArray() {
        for (int i = 0; i < N; ++i) {
            readersNumberLock[i] = new Object();
        }
    }

    public void write(int p, int x, int y) {
        if (legalIndex(p)) {
            synchronized (readersNumberLock[p]) {
                while (readersNumber[p] > 0) {
                    try {
                        readersNumberLock[p].wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                this.x[p] = x;
                this.y[p] = y;
            }
        }
    }

    public int[] read(int p) {
        int[] result = null;

        if (legalIndex(p)) {
            result = new int[2];

            synchronized (readersNumberLock[p]) {
                ++readersNumber[p];
            }

            result[0] = x[p];
            result[1] = y[p];

            synchronized (readersNumberLock[p]) {
                --readersNumber[p];

                if (0 == readersNumber[p]) {
                    readersNumberLock[p].notifyAll();
                }
            }
        }

        return result;
    }
}
