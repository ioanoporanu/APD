import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.stream.Collectors;

//clasa care creeaza task-urile neceasare pentru etapa de Map
public class CreateMapTasks {
    private final ReadInpData reader;
    private final ExecutorService tpe;
    private final Map<String, ArrayList<TaskMapResult>> taskMapList;

    //constructorul care contine input-ul si lista de rezultate si executorService-ul
    public CreateMapTasks(ReadInpData reader, ExecutorService tpe){
        this.reader = reader;
        this.tpe = tpe;
        this.taskMapList = new HashMap<>();
    }

    public Map<String, ArrayList<TaskMapResult>> getTaskMapList() {
        return taskMapList;
    }

    //in aceasta metoda se genereaza task-urile pentru un fisier dat ca parametru
    public void createFileTasks (ExecutorService tpe, String fileName) throws ExecutionException, InterruptedException {
        int offset = 0;
        File file = new File(fileName);
        int size = (int) file.length();
        ArrayList<TaskMapResult> taskList = new ArrayList<>();
        for(int i = 0; i < size / reader.getDim(); i++) {
            Future<TaskMapResult> taskResult = tpe.submit(new TaskMap(fileName, offset, reader.getDim()));
            //se colecteaza rezultatul pentru acest task
            taskList.add(taskResult.get());
            offset += reader.getDim();
        }

        if(size % reader.getDim() != 0) {
            Future<TaskMapResult> taskResult = tpe.submit(new TaskMap(fileName, offset, size % reader.getDim()));
            taskList.add(taskResult.get());
        }

        //se colecteaza rezultatul pentru acest task
        taskList = (ArrayList<TaskMapResult>) taskList.stream().filter(Objects::nonNull).collect(Collectors.toList());
        taskMapList.put(fileName, taskList);
    }

    //in aceasta metoda se genereaza task-urile pentru toate fisierele din input
    public void createMapTasks() throws ExecutionException, InterruptedException {
        for(int i = 0; i < reader.getNrOfFiles(); i++) {
            createFileTasks(tpe, reader.getFiles().get(i));
        }
    }
}
