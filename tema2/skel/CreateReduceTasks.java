import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;

//clasa care creeaza task-urile pentru etape de Reduce
public class CreateReduceTasks {
    private final Map<String, ArrayList<TaskMapResult>> taskMapResults;
    private final ArrayList<TaskReduceResult> taskReduceResults;
    ExecutorService tpe;

    //se dau in constructor rezultatele de la etapa de Map si serviceExecutorul
    public CreateReduceTasks(Map<String, ArrayList<TaskMapResult>> taskMapResults, ExecutorService tpe) {
        this.taskMapResults = taskMapResults;
        this.tpe = tpe;
        this.taskReduceResults = new ArrayList<>();
    }

    public ArrayList<TaskReduceResult> getTaskReduceResults() {
        return taskReduceResults;
    }

    //metoda care creeaza task-urile pentru etapa de Reduce
    public void createReduceTasks(ArrayList<String> files) throws ExecutionException, InterruptedException {
        for(String file : files){
            Future<TaskReduceResult> taskResult = tpe.submit(new TaskReduce(taskMapResults.get(file), file));
            taskReduceResults.add(taskResult.get());
        }
    }
}
