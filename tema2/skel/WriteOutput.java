import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.stream.Collectors;

import static java.lang.Double.compare;
//clasa care scrie in fisier rezultatele finale
public class WriteOutput {
    public static void writeOutput(String outFilename, ArrayList<TaskReduceResult> taskReduceResultsList) throws IOException {
        StringBuilder text = new StringBuilder();
        BufferedWriter writer = new BufferedWriter(new FileWriter(outFilename));

        //se soreteaza lista de rezulatte reduce
        taskReduceResultsList = (ArrayList<TaskReduceResult>) taskReduceResultsList.stream()
                .sorted((p1, p2) -> compare(p2.getRank(), p1.getRank()))
                .collect(Collectors.toList());

        //se formeaza string-ul ce trebuie scris in fisier
        for (TaskReduceResult t : taskReduceResultsList) {
            StringBuilder currText = new StringBuilder();
            currText.append(t.getFileName());
            currText.append(",");
            currText.append(String.format("%.2f", t.getRank()));
            currText.append(",");
            currText.append(t.getMaxLength());
            currText.append(",");
            currText.append(t.getNoOfWords()).append("\n");
            String delete = "../tests/files/";
            currText.delete(0, delete.length());

            text.append(currText);
        }
        text.deleteCharAt(text.length() - 1);

        //se scrie in fisier
        writer.write(text.toString());
        writer.close();
    }
}
