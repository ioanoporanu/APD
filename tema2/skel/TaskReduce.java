import java.util.*;
import java.util.concurrent.Callable;


public class TaskReduce implements Callable<TaskReduceResult> {
    private final ArrayList<TaskMapResult> taskMapResults;
    private final String fileName;

    public TaskReduce(ArrayList<TaskMapResult> taskMapResults, String fileName){
        this.taskMapResults = taskMapResults;
        this.fileName = fileName;
    }

    //calculeaza elementul din sirul fibonacci de la o anumita pozitie
    public int fibonacci(int length) {
        if(length <= 1) {
            return length;
        }

        int fib = 1;
        int prevFib = 1;

        for(int i = 2; i < length; i++) {
            int temp = fib;
            fib += prevFib;
            prevFib = temp;
        }
        return fib;
    }

    public TaskReduceResult call() throws Exception {
        HashMap<Integer, Integer> mapWords = new HashMap<>();

        //se unesc dictionarele
        for (TaskMapResult mapRes : taskMapResults){
            mapRes.getDictionary().forEach(
                    (key, value) -> mapWords.merge( key, value, (v1, v2) -> v1 + v2)
                );
        }

        //se calculeaza lungimea maxima pentru fisier
        int maxLength = 0;
        for (Map.Entry<Integer, Integer> entry : mapWords.entrySet()) {
            if(maxLength < entry.getKey()){
                maxLength = entry.getKey();
            }
        }

        //se calculeaza nr de cuvinte din fisier
        int nrWwords = 0;
        for (Map.Entry<Integer, Integer> entry : mapWords.entrySet()) {
            nrWwords += entry.getValue();
        }

        //se calculeaza rank-ul
        double rank = 0;
        for (Map.Entry<Integer, Integer> entry : mapWords.entrySet()) {
            rank += fibonacci(entry.getKey() + 1) * entry.getValue();
        }
        rank = rank / nrWwords;

        //se returneaza rezultatul
        return new TaskReduceResult(fileName, rank, maxLength, mapWords.get(maxLength));
    }
}
