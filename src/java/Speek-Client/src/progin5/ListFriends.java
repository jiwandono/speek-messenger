package progin5;

/**
 *
 * @author Ananti
 */
public class ListFriends {
    private String name;
    private int status;

     public ListFriends() {
        this.name = "";
        this.status = 0;
    }

    public ListFriends(String name, int status) {
        this.name = name;
        this.status = status;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }
}
