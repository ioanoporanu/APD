import java.util.ArrayList;
import java.util.HashMap;
//contine rezultatul de la etaoa de Map
public class TaskMapResult {
    private final HashMap<Integer, Integer> dictionary;
    private final ArrayList<String> maxLengthWords;

    public TaskMapResult(HashMap<Integer, Integer> dictionary, ArrayList<String> maxLengthWords){
        this.dictionary = dictionary;
        this.maxLengthWords = maxLengthWords;
    }

    public HashMap<Integer, Integer> getDictionary() {
        return dictionary;
    }
}
