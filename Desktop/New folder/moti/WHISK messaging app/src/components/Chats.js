import React,{useRef , useState, useEffect } from 'react';
import { useHistory } from 'react-router-dom';
import { ChatEngine } from 'react-chat-engine';
import { auth } from '../firebase';

import { useAuth } from '../contexts/AuthContext';
import axios from 'axios';

const Chats = () => {
    const history = useHistory();
    const { user } = useAuth();
    const [loading, setLoading] = useState(true);

    console.log(user);

    const handleLogout = async () => {
        await auth.signOut();

        history.push('/');

    }

    const getFile = async(url) => {
        const response = await fetch(url);
        const data = await response.blob();

        return new File([data], "userPhoto.jpg", { type: 'image.jpg' })
    }
    
    useEffect (() => {
        if (!user) {
            history.push('/');

            return;
        }

        axios.get('https://api.chatengine.io/users/me', {
            headers: {
                "project-id": "997ada12-86d1-4c06-8839-2bc84ea253d5",
                "user-name": user.email,
                "user-secret": user.uid, 
            }
        })
        .then(() => {
            setLoading(false);
        })
        .catch(() => {
            let formdata = new FormData();
            formdata.append('email', user.email);
            formdata.append('username', user.displayName);
            formdata.append('secret', user.uid);

            getFile(user.photoURL)
                .then((avatar) => {
                    formdata.append('avatar', avatar, avatar.name);

                    axios.post('https://api.chatengine.io/users',
                        formData, 
                        { headers: {"private-key": "59354279-5291-4d45-bd98-f7d6697dce81"} }
                    )
                    .then(() => setLoading(false))
                    .catch((error) => console.log(error))
            })
        
    })
    }, [user, history]);

    if(!user || loading) return 'Loading...';

    return (
        <div className="chats-page">
            <div className='"nav-bar'>
                <div className="logo-tab">
                    WhiskChat
                </div>
                <div onClick={handleLogout} className="logout-tab">
                    Logout
                </div>
            </div>
            
            <ChatEngine
                height="calc(100vh - 66px)"
                projectID="997ada12-86d1-4c06-8839-2bc84ea253d5" // is moved to env variables in the later stage
                userName= {user.email}
                userSecret= {user.uid}
            />
        </div>    
    );
}

export default Chats;