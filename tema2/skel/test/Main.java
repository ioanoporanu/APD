package test;

import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Main {
    public static void main(String[] args) throws IOException {

        ArrayList<Integer> ints = new ArrayList<>();
        test test = new test(ints);
        test.suckDick();
        System.out.println(ints.size());
    }
}
